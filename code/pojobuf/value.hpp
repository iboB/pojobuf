// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "value_type.hpp"
#include "pl_tag.hpp"
#include "docstore.hpp"

#include <bit>
#include <span>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <string_view>

#include <splat/inline.h>
#include <splat/unreachable.h>

namespace pojobuf {

class value {
public:
    value() noexcept : m_tag(pl_tag::undefined), m_data_ptr(nullptr), m_byte_ptr(nullptr) {}

    explicit value(int64_t payload, const int64_t* parent_data_ptr, const char* byte_ptr)
        : m_tag(docstore::get_tag_from_payload(payload))
        , m_data_ptr(parent_data_ptr + docstore::get_offset_from_payload(payload))
        , m_byte_ptr(byte_ptr)
    {}

    value(const value&) noexcept = default;
    value& operator=(const value&) noexcept = default;

    pl_tag tag() const noexcept { return m_tag; }
    value_type type() const noexcept { return value_type(m_tag); }
    const int64_t* data_ptr() const noexcept { return m_data_ptr; }
    const char* byte_ptr() const noexcept { return m_byte_ptr; }

    bool get_boolean_value() const noexcept {
        assert(type().is_boolean());
        return type().is_true();
    }

    int32_t get_int32_value() const noexcept {
        assert(type().is_integer());
        return int32_t(*m_data_ptr);
    }

    int64_t get_int64_value() const noexcept {
        assert(type().is_integer());
        return *m_data_ptr;
    }

    int64_t get_integer_value() const noexcept {
        assert(type().is_integer());
        return *m_data_ptr;
    }

    double get_real_value() const noexcept {
        assert(type().is_real());
        static_assert(sizeof(double) == sizeof(int64_t));
        return std::bit_cast<double>(*m_data_ptr);
    }

    double get_as_double() const noexcept {
        assert(type().is_number());
        if (type().is_integer()) {
            return double(*m_data_ptr);
        }
        else {
            return get_real_value();
        }
    }

    float get_as_float() const noexcept {
        return float(get_as_double());
    }

    int64_t get_as_integer() const noexcept {
        assert(type().is_number());
        if (type().is_integer()) {
            return *m_data_ptr;
        }
        else {
            return int64_t(get_real_value());
        }
    }

    size_t get_buffer_length() const noexcept {
        assert(type().is_buffer());
        return size_t(m_data_ptr[1] - m_data_ptr[0]);
    }

    std::span<const char> get_buffer_value() const noexcept {
        assert(type().is_buffer());
        return std::span<const char>(m_byte_ptr + m_data_ptr[0], m_byte_ptr + m_data_ptr[1]);
    }

    std::span<const std::byte> get_blob_value() const noexcept {
        assert(type().is_buffer());
        return as_bytes(get_buffer_value());
    }

    size_t get_string_length() const noexcept {
        return get_buffer_length();
    }

    std::string_view get_string_value() const noexcept {
        assert(type().is_buffer());
        return std::string_view(m_byte_ptr + m_data_ptr[0], m_byte_ptr + m_data_ptr[1]);
    }

    size_t get_compound_length() const noexcept {
        assert(type().is_compound());
        return size_t(*m_data_ptr);
    }

    size_t get_array_length() const noexcept {
        assert(type().is_array());
        return get_compound_length();
    }

    value get_array_element(size_t index) const noexcept {
        assert(type().is_array());
        assert(index < get_compound_length());
        const auto pl = m_data_ptr[1 + index];
        return value(pl, m_data_ptr, m_byte_ptr);
    }

    value get_array_element_safe(size_t index) const noexcept {
        assert(type().is_array());
        if (index >= get_compound_length()) {
            return {};
        }
        return get_array_element(index);
    }

    size_t get_object_length() const noexcept {
        assert(type().is_object());
        return get_compound_length();
    }

    std::string_view get_object_key(size_t index) const noexcept {
        assert(type().is_object());
        assert(index < get_compound_length());
        const auto& r = get_object_elems()[index];
        return std::string_view(m_byte_ptr + r.key_start, m_byte_ptr + r.key_end);
    }

    value get_object_value(size_t index) const noexcept {
        assert(type().is_object());
        assert(index < get_compound_length());
        const auto& r = get_object_elems()[index];
        return value(r.value_payload, m_data_ptr, m_byte_ptr);
    }

    using kv = std::pair<std::string_view, value>;

    kv get_object_element(size_t index) const noexcept {
        assert(type().is_object());
        assert(index < get_compound_length());
        const auto& r = get_object_elems()[index];
        return {
            std::string_view(m_byte_ptr + r.key_start, m_byte_ptr + r.key_end),
            value(r.value_payload, m_data_ptr, m_byte_ptr)
        };
    }

    kv get_object_element_safe(size_t index) const noexcept {
        assert(type().is_object());
        if (index >= get_compound_length()) {
            return {};
        }
        return get_object_element(index);
    }

    // sorted objects are the rare case (most objects aren't big enough to be sorted; see
    // buf_builder::should_sort_object). keeping this path out of find_object_key proper, and
    // never inlining it, keeps the common (linear scan) case small enough to always inline
    FORCE_INLINE size_t find_object_key(std::string_view key) const noexcept {
        using namespace docstore;
        assert(type().is_object());

        const auto length = get_compound_length();
        const auto elems = get_object_elems();

        if (m_tag == pl_tag::sorted_object) [[unlikely]] {
            return find_sorted_object_key(key, elems, length);
        }

        for (size_t i = 0; i < length; ++i) {
            const auto& r = elems[i];
            auto len = size_t(r.key_end - r.key_start);
            if (len == key.size() && std::memcmp(m_byte_ptr + r.key_start, key.data(), len) == 0) {
                return i;
            }
        }
        return length;
    }

    value get_object_value_safe(std::string_view key) const noexcept {
        assert(type().is_object());
        const auto index = find_object_key(key);
        if (index >= get_compound_length()) {
            return {};
        }
        return get_object_value(index);
    }

private:
    pl_tag m_tag;
    const int64_t* m_data_ptr;
    const char* m_byte_ptr;

    const docstore::object_elem* get_object_elems() const noexcept {
        assert(type().is_object());
        return reinterpret_cast<const docstore::object_elem*>(m_data_ptr + 1);
    }

    NOINLINE size_t find_sorted_object_key(
        std::string_view key,
        const docstore::object_elem* elems,
        size_t length
    ) const noexcept {
        using namespace docstore;

        auto key_eq = [&](const object_elem& r) FORCE_INLINE_LAMBDA {
            auto len = size_t(r.key_end - r.key_start);
            if (len != key.size()) return false;
            return std::memcmp(m_byte_ptr + r.key_start, key.data(), len) == 0;
        };

        const auto it = std::lower_bound(elems, elems + length, key, object_key_cmp{m_byte_ptr});
        if (it == elems + length) {
            return length;
        }
        if (key_eq(*it)) {
            return size_t(it - elems);
        }
        return length;
    }
};

} // namespace pojobuf
