// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "value.hpp"

namespace pojobuf {

class value::object_elems {
    value m_value;
public:
    explicit object_elems(const value& val)
        : m_value(val)
    {
        assert(val.type().is_object());
    }

    size_t size() const noexcept { return m_value.object_length(); }

    value operator[](std::string_view key) const noexcept {
        return m_value.object_value_at_key(key);
    }

    value find(std::string_view key) const noexcept {
        return m_value.object_value_at_key_safe(key);
    }

    class iterator {
        const bufutil::object_elem* m_ptr;
        const int64_t* m_data_ptr;
        const char* m_byte_ptr;
    public:
        iterator(const bufutil::object_elem* ptr, const int64_t* data_ptr, const char* byte_ptr)
            : m_ptr(ptr)
            , m_data_ptr(data_ptr)
            , m_byte_ptr(byte_ptr)
        {}

        iterator& operator++() noexcept {
            ++m_ptr;
            return *this;
        }
        value::kv operator*() const noexcept {
            const auto& r = *m_ptr;
            return {
                std::string_view(m_byte_ptr + r.key_start, m_byte_ptr + r.key_end),
                value(r.value_payload, m_data_ptr, m_byte_ptr)
            };
        }

        auto operator<=>(const iterator& other) const noexcept {
            // no need to compare everything
            return m_ptr <=> other.m_ptr;
        }
        bool operator==(const iterator& other) const noexcept {
            return operator<=>(other) == 0;
        }
    };

    iterator begin() const noexcept {
        return {m_value.get_object_elems(), m_value.m_data_ptr, m_value.m_byte_ptr};
    }
    iterator end() const noexcept {
        return {m_value.get_object_elems() + size(), m_value.m_data_ptr, m_value.m_byte_ptr};
    }
};

value::object_elems value::object_elements() const noexcept {
    return object_elems(*this);
}

} // namespace pojobuf
