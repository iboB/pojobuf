// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "value.hpp"

namespace pojobuf {

class value::array_elems {
    const int64_t* m_data_ptr;
    const char* m_byte_ptr;
public:
    explicit array_elems(const value& val)
        : m_data_ptr(val.m_data_ptr)
        , m_byte_ptr(val.m_byte_ptr)
    {
        assert(val.type().is_array());
    }

    size_t size() const noexcept { return size_t(*m_data_ptr); }

    value operator[](size_t i) const noexcept {
        assert(i < size());
        const auto pl = m_data_ptr[1 + i];
        return value(pl, m_data_ptr, m_byte_ptr);
    }

    class iterator {
        const int64_t* m_ptr;
        const int64_t* m_data_ptr;
        const char* m_byte_ptr;
    public:
        iterator(const int64_t* ptr, const int64_t* data_ptr, const char* byte_ptr)
            : m_ptr(ptr)
            , m_data_ptr(data_ptr)
            , m_byte_ptr(byte_ptr)
        {}

        iterator& operator++() noexcept {
            ++m_ptr;
            return *this;
        }
        value operator*() const noexcept {
            return value(*m_ptr, m_data_ptr, m_byte_ptr);
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
        return {m_data_ptr + 1, m_data_ptr, m_byte_ptr};
    }
    iterator end() const noexcept {
        return {m_data_ptr + size() + 1, m_data_ptr, m_byte_ptr};
    }
};

value::array_elems value::array_elements() const noexcept {
    return array_elems(*this);
}

} // namespace pojobuf
