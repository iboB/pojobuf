// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "value_type.hpp"
#include "pl_tag.hpp"
#include "bufutil.hpp"

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
        : m_tag(bufutil::get_tag_from_payload(payload))
        , m_data_ptr(parent_data_ptr + bufutil::get_offset_from_payload(payload))
        , m_byte_ptr(byte_ptr)
    {}

    value(const value&) noexcept = default;
    value& operator=(const value&) noexcept = default;

    pl_tag tag() const noexcept { return m_tag; }
    value_type type() const noexcept { return value_type(m_tag); }
    const int64_t* data_ptr() const noexcept { return m_data_ptr; }
    const char* byte_ptr() const noexcept { return m_byte_ptr; }

    FORCE_INLINE bool boolean_value() const noexcept {
        assert(type().is_boolean());
        return type().is_true();
    }
    FORCE_INLINE int32_t int32_value() const noexcept {
        assert(type().is_integer());
        return int32_t(*m_data_ptr);
    }
    FORCE_INLINE int64_t int64_value() const noexcept {
        assert(type().is_integer());
        return *m_data_ptr;
    }
    FORCE_INLINE int64_t integer_value() const noexcept {
        return int64_value();
    }

    // be careful with this
    // many parsers (ie JSON) will make an integer when they see a whole number
    // asserting that a number is real here, will only work for fractional ones
    // use this when you know what you're doing. Some examples:
    //  * you've checked that type is real
    //  * you know for a fact this is not a whole number
    //  * you know that the produced of this value has put a real number here even if whole
    FORCE_INLINE double real_value() const noexcept {
        assert(type().is_real());
        static_assert(sizeof(double) == sizeof(int64_t));
        return std::bit_cast<double>(*m_data_ptr);
    }

    FORCE_INLINE double real_value_safe() const noexcept {
        assert(type().is_number());
        if (type().is_integer()) {
            return double(*m_data_ptr);
        }
        else {
            return real_value();
        }
    }
    FORCE_INLINE double f64_value() const noexcept {
        return real_value_safe();
    }
    FORCE_INLINE float f32_value() const noexcept {
        return float(real_value_safe());
    }

    // truncates fractional values
    FORCE_INLINE int64_t integer_value_safe() const noexcept {
        assert(type().is_number());
        if (type().is_integer()) {
            return *m_data_ptr;
        }
        else {
            return int64_t(real_value());
        }
    }

    FORCE_INLINE size_t buffer_size() const noexcept {
        assert(type().is_buffer());
        return size_t(m_data_ptr[1] - m_data_ptr[0]);
    }

    FORCE_INLINE std::span<const char> buffer_value() const noexcept {
        assert(type().is_buffer());
        return std::span<const char>(m_byte_ptr + m_data_ptr[0], m_byte_ptr + m_data_ptr[1]);
    }

    FORCE_INLINE size_t blob_size() const noexcept {
        return buffer_size();
    }

    FORCE_INLINE std::span<const std::byte> blob_value() const noexcept {
        assert(type().is_buffer());
        return as_bytes(buffer_value());
    }

    FORCE_INLINE size_t string_length() const noexcept {
        return buffer_size();
    }

    FORCE_INLINE std::string_view string_value() const noexcept {
        assert(type().is_buffer());
        return std::string_view(m_byte_ptr + m_data_ptr[0], m_byte_ptr + m_data_ptr[1]);
    }

    FORCE_INLINE size_t compound_length() const noexcept {
        assert(type().is_compound());
        return size_t(*m_data_ptr);
    }

    FORCE_INLINE size_t array_length() const noexcept {
        assert(type().is_array());
        return compound_length();
    }

    FORCE_INLINE value array_element_at(size_t index) const noexcept {
        assert(type().is_array());
        assert(index < compound_length());
        const auto pl = m_data_ptr[1 + index];
        return value(pl, m_data_ptr, m_byte_ptr);
    }

    // return undefined when out of bounds
    FORCE_INLINE value array_element_at_safe(size_t index) const noexcept {
        assert(type().is_array());
        if (index >= compound_length()) {
            return {};
        }
        return array_element_at(index);
    }

    FORCE_INLINE size_t object_length() const noexcept {
        assert(type().is_object());
        return compound_length();
    }

    FORCE_INLINE std::string_view object_key_at(size_t index) const noexcept {
        assert(type().is_object());
        assert(index < compound_length());
        const auto& r = get_object_elems()[index];
        return std::string_view(m_byte_ptr + r.key_start, m_byte_ptr + r.key_end);
    }

    FORCE_INLINE value object_value_at(size_t index) const noexcept {
        assert(type().is_object());
        assert(index < compound_length());
        const auto& r = get_object_elems()[index];
        return value(r.value_payload, m_data_ptr, m_byte_ptr);
    }

    FORCE_INLINE value object_value_at_safe(size_t index) const noexcept {
        assert(type().is_object());
        if (index >= compound_length()) {
            return {};
        }
        return object_value_at(index);
    }

    using kv = std::pair<std::string_view, value>;

    FORCE_INLINE kv object_element_at(size_t index) const noexcept {
        assert(type().is_object());
        assert(index < compound_length());
        const auto& r = get_object_elems()[index];
        return {
            std::string_view(m_byte_ptr + r.key_start, m_byte_ptr + r.key_end),
            value(r.value_payload, m_data_ptr, m_byte_ptr)
        };
    }

    FORCE_INLINE kv object_element_at_safe(size_t index) const noexcept {
        assert(type().is_object());
        if (index >= compound_length()) {
            return {};
        }
        return object_element_at(index);
    }

    // return index of key or object size if it doesn't exist
    FORCE_INLINE size_t find_object_key(std::string_view key) const noexcept {
        using namespace bufutil;
        assert(type().is_object());

        const auto length = compound_length();
        const auto elems = get_object_elems();

        if (m_tag == pl_tag::sorted_object) [[unlikely]] {
            // sorted objects are the rare case (most objects aren't big enough to be sorted; see
            // buf_builder::should_sort_object). keeping this path out of find_object_key proper, and
            // never inlining it, keeps the common (linear scan) case small enough to always inline
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

    FORCE_INLINE value object_value_at_key(std::string_view key) const noexcept {
        assert(type().is_object());
        const auto index = find_object_key(key);
        assert(index < compound_length());
        return object_value_at(index);
    }

    FORCE_INLINE value object_value_at_key_safe(std::string_view key) const noexcept {
        assert(type().is_object());
        const auto index = find_object_key(key);
        return object_value_at_safe(index);
    }

    class array_elems;
    array_elems array_elements() const noexcept;

    class object_elems;
    object_elems object_elements() const noexcept;

private:
    pl_tag m_tag;
    const int64_t* m_data_ptr;
    const char* m_byte_ptr;

    FORCE_INLINE const bufutil::object_elem* get_object_elems() const noexcept {
        assert(type().is_object());
        return reinterpret_cast<const bufutil::object_elem*>(m_data_ptr + 1);
    }

    NOINLINE size_t find_sorted_object_key(
        std::string_view key,
        const bufutil::object_elem* elems,
        size_t length
    ) const noexcept {
        using namespace bufutil;

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
