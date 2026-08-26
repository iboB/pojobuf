// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "value.hpp"
#include "doc_alloc_strategy.hpp"
#include "bits/pod_vector.hpp"
#include <splat/unreachable.h>

namespace pojobuf {

struct no_buf {
    constexpr bool empty() const noexcept { return true; }
    constexpr const char* data() const noexcept { return nullptr; }
};

template <typename ByteBuf = no_buf>
class document {
    bits::pod_vector m_buffer;
    ByteBuf m_byte_buf;
    const char* m_x_byte_ptr;
    int64_t m_root_payload;
public:
    document() = default;

    explicit document(bits::pod_vector&& buffer, ByteBuf&& byte_buf, const char* x_byte_ptr, int64_t root_payload)
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

    const bits::pod_vector& buffer() const noexcept { return m_buffer; }
    const ByteBuf& byte_buf() const noexcept { return m_byte_buf; }

    int64_t root_payload() const noexcept { return m_root_payload; }

    doc_alloc_strategy alloc_strategy() const noexcept {
        using enum doc_alloc_strategy;
        if (!m_x_byte_ptr){
            if (m_byte_buf.empty()) {
                return embed_bytes_in_data;
            }
            else {
                return take_source;
            }
        }
        else {
            return external_mutable_source;
        }
    }

    const char* byte_ptr() const noexcept {
        using enum doc_alloc_strategy;
        switch (alloc_strategy()) {
        case embed_bytes_in_data:
            return reinterpret_cast<const char*>(m_buffer.data());
        case take_source:
            return m_byte_buf.data();
        case external_mutable_source:
            return m_x_byte_ptr;
        default:
            SPLAT_UNREACHABLE();
        }
    }

    value root() const noexcept {
        return value(m_root_payload, m_buffer.data(), byte_ptr());
    }
};

} // namespace pojobuf
