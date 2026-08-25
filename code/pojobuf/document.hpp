// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "value.hpp"
#include "bits/pod_vector.hpp"

namespace pojobuf {

class document {
    bits::pod_vector m_buffer;
    int64_t m_root_payload = 0;
public:
    document() = default;

    explicit document(bits::pod_vector&& buffer, int64_t root_payload)
        : m_buffer(std::move(buffer))
        , m_root_payload(root_payload)
    {}

    explicit document(const document&) = default;
    document& operator=(const document&) = delete; // no accidental copying, use copy() instead
    document copy() const {
        return document(*this);
    }

    document(document&& other) noexcept
        : m_buffer(std::move(other.m_buffer))
        , m_root_payload(other.m_root_payload)
    {
        other.m_root_payload = 0;
    }
    document& operator=(document&& other) noexcept {
        if (this != &other) {
            m_buffer = std::move(other.m_buffer);
            m_root_payload = other.m_root_payload;
            other.m_root_payload = 0;
        }
        return *this;
    }

    value root() const noexcept {
        return value(m_root_payload, m_buffer.data(), reinterpret_cast<const char*>(m_buffer.data()));
    }
};

} // namespace pojobuf
