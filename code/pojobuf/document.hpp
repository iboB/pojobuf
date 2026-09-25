// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "value.hpp"
#include <itlib/pod_vector.hpp>
#include <splat/unreachable.h>

namespace pojobuf {

struct no_buf {
    constexpr no_buf() = default;
    template <typename T>
    constexpr explicit no_buf(const T&) {}
    constexpr bool empty() const noexcept { return true; }
    constexpr const char* data() const noexcept { return nullptr; }
};

template <typename ByteBuf = no_buf>
class document {
    itlib::pod_vector_noinit<int64_t> m_buffer; // structure (and optionally strings)
    ByteBuf m_byte_buf; // optional owned string buffer (likely the parsed source)
    const char* m_x_byte_ptr; // optional external string pointer
    int64_t m_root_payload;
public:
    document() = default;

    explicit document(itlib::pod_vector_noinit<int64_t>&& buffer, ByteBuf&& byte_buf, const char* x_byte_ptr, int64_t root_payload)
        : m_buffer(std::move(buffer))
        , m_byte_buf(std::move(byte_buf))
        , m_x_byte_ptr(x_byte_ptr)
        , m_root_payload(root_payload)
    {}

    explicit document(const document&) = default;
    document& operator=(const document&) = delete; // no accidental copying, use copy() instead
    document copy() const {
        return document(*this);
    }

    document(document&& other) noexcept
        : m_buffer(std::move(other.m_buffer))
        , m_byte_buf(std::move(other.m_byte_buf))
        , m_x_byte_ptr(other.m_x_byte_ptr)
        , m_root_payload(other.m_root_payload)
    {
        other.m_root_payload = 0;
    }
    document& operator=(document&& other) noexcept {
        if (this != &other) {
            m_buffer = std::move(other.m_buffer);
            m_byte_buf = std::move(other.m_byte_buf);
            m_x_byte_ptr = other.m_x_byte_ptr;
            m_root_payload = other.m_root_payload;
            other.m_root_payload = 0;
        }
        return *this;
    }

    bool empty() const noexcept { return m_buffer.empty(); }

    const itlib::pod_vector_noinit<int64_t>& buffer() const noexcept { return m_buffer; }
    const ByteBuf& byte_buf() const noexcept { return m_byte_buf; }

    int64_t root_payload() const noexcept { return m_root_payload; }

    bool has_separate_byte_buf() const noexcept {
        return !m_byte_buf.empty();
    }

    // if true, the document uses an external byte buffer and does not own all data
    // it is then the responsibility of the user to ensure that the external buffer remains valid
    // for the lifetime of the document
    // otherwise, the document owns all needed data and can be freely moved and copied around
    bool uses_external_byte_buf() const noexcept {
        return !!m_x_byte_ptr;
    }

    const char* byte_ptr() const noexcept {
        if (uses_external_byte_buf()) {
            return m_x_byte_ptr;
        }
        else if (has_separate_byte_buf()) {
            return m_byte_buf.data();
        }
        else {
            return reinterpret_cast<const char*>(m_buffer.data());
        }
    }

    value root() const noexcept {
        return value(m_root_payload, m_buffer.data(), byte_ptr());
    }
};

} // namespace pojobuf
