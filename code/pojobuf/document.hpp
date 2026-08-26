// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "value.hpp"
#include "bits/pod_vector.hpp"
#include <type_traits>

namespace pojobuf {

struct embedded_bytes {
    const char* data() const noexcept { return nullptr; }
};
struct external_bytes {
    const char* ptr = nullptr;
    bool empty() const noexcept { return !ptr; }
    const char* data() const noexcept { return ptr; }
};

template <typename ByteBuf = embedded_bytes>
class document {
    bits::pod_vector m_buffer;
    ByteBuf m_byte_buf;
    int64_t m_root_payload = 0;
public:
    document() = default;

    explicit document(bits::pod_vector&& buffer, ByteBuf&& byte_buf, int64_t root_payload)
        : m_buffer(std::move(buffer))
        , m_byte_buf(std::move(byte_buf))
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
        , m_root_payload(other.m_root_payload)
    {
        other.m_root_payload = 0;
    }
    document& operator=(document&& other) noexcept {
        if (this != &other) {
            m_buffer = std::move(other.m_buffer);
            m_byte_buf = std::move(other.m_byte_buf);
            m_root_payload = other.m_root_payload;
            other.m_root_payload = 0;
        }
        return *this;
    }

    const bits::pod_vector& buffer() const noexcept { return m_buffer; }
    const ByteBuf& byte_buf() const noexcept { return m_byte_buf; }
    const int64_t root_payload() const noexcept { return m_root_payload; }

    bool has_embedded_bytes() const noexcept {
        if constexpr (std::is_same_v<ByteBuf, embedded_bytes>) {
            return true;
        }
        else {
            return m_byte_buf.empty();
        }
    }

    const char* byte_ptr() const noexcept {
        if (has_embedded_bytes()) {
            return reinterpret_cast<const char*>(m_buffer.data());
        }
        else {
            return m_byte_buf.data();
        }
    }

    value root() const noexcept {
        return value(m_root_payload, m_buffer.data(), byte_ptr());
    }
};

} // namespace pojobuf
