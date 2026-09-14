// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "bytes_range.hpp"
#include "bits/imath.hpp"
#include <splat/inline.h>
#include <cstdint>
#include <cassert>

namespace pojobuf::alloc {

class mutable_source_byte_alloc {
    char* m_text;
public:
    explicit mutable_source_byte_alloc(char* text) noexcept
        : m_text(text)
    {}

    const char* get_byte_ptr() const noexcept {
        return m_text;
    }

    bytes_range push_internal_string(const char* begin, const char* end) noexcept {
        assert(begin > m_text);
        return bytes_range{begin - m_text, end - m_text};
    }

    struct piecewise_string_builder {
        const char* simple_begin;
        char* p;
        FORCE_INLINE void push(char c) noexcept {
            *p++ = c;
        }
    };
    piecewise_string_builder get_piecewise_string_builder(const char* simple_begin, const char* begin) noexcept {
        return piecewise_string_builder{simple_begin, const_cast<char*>(begin)};
    }
    bytes_range push_string(const piecewise_string_builder& psb) noexcept {
        return push_internal_string(psb.simple_begin, psb.p);
    }
};

template <typename DataAlloc>
class valuebuf_byte_alloc {
    DataAlloc& m_data_alloc;
public:
    explicit valuebuf_byte_alloc(DataAlloc& data_alloc) noexcept
        : m_data_alloc(data_alloc)
    {}

    char* get_byte_ptr() const noexcept {
        return reinterpret_cast<char*>(m_data_alloc.get_value_buffer_ptr());
    }

    bytes_range push_string(const char* begin, const char* end) {
        const int64_t begin_offset = m_data_alloc.get_value_offset() * sizeof(int64_t);
        const int64_t length = end - begin;

        if (length == 0) [[unlikely]] {
            return bytes_range{begin_offset, begin_offset};
        }

        const auto value_length = bits::divide_round_up(length, int64_t(sizeof(int64_t)));
        auto ptr = m_data_alloc.alloc_value(value_length);
        std::memcpy(ptr, begin, length);
        return bytes_range{begin_offset, begin_offset + length};
    }

    bytes_range push_internal_string(const char* begin, const char* end) {
        return push_string(begin, end);
    }

    struct piecewise_string_builder {
        char* p;
        int64_t begin_offset;
        int64_t length;
        DataAlloc& data_alloc;
        FORCE_INLINE void push(char c) {
            if (length % sizeof(int64_t) == 0) {
                p = reinterpret_cast<char*>(data_alloc.alloc_value());
            }
            *p++ = c;
            ++length;
        }
    };
    piecewise_string_builder get_piecewise_string_builder(const char* simple_begin, const char* begin) {
        auto range = push_string(simple_begin, begin);
        return piecewise_string_builder{
            .p = get_byte_ptr() + range.end,
            .begin_offset = range.begin,
            .length = range.end - range.begin,
            .data_alloc = m_data_alloc
        };
    }
    bytes_range push_string(const piecewise_string_builder& psb) noexcept {
        return bytes_range{psb.begin_offset, psb.begin_offset + psb.length};
    }
};

// allocate and use the front of the buffer
// use the back of the buffer as a scratch space for compound types
// assume that the buffer is large enough to hold all values and payloads
class single_buf_nocheck_data_alloc {
public:
    static constexpr bool is_single_buf = true;

    int64_t* buffer;
    int64_t* head;
    int64_t* tail;

    template <typename C>
    static single_buf_nocheck_data_alloc from_container(C& c) noexcept {
        single_buf_nocheck_data_alloc ret;
        ret.buffer = c.data();
        ret.head = ret.buffer;
        ret.tail = ret.buffer + c.size();
        return ret;
    }

    int64_t* get_value_buffer_ptr() const noexcept {
        return buffer;
    }

    int64_t get_value_offset() const noexcept {
        return head - buffer;
    }
    int64_t* alloc_value(size_t n = 1) noexcept {
        auto ret = head;
        head += n;
        return ret;
    }

    int64_t get_cur_payload() const noexcept {
        return *tail;
    }
    int64_t get_cur_payload_offset() const noexcept {
        return int64_t(tail - buffer);
    }
    int64_t* alloc_payload() noexcept {
        return --tail;
    }
};

// allocate and use the front of the buffer for values
// use another buffer for scratch space for payloads
// assume that both buffers are large enough to hold all values and payloads
struct multi_buf_nocheck_data_alloc {
    static constexpr bool is_single_buf = false;

    int64_t* val_buffer;
    int64_t* pval;

    int64_t* pl_buffer;
    int64_t* ppl;

    template <typename C1, typename C2>
    static multi_buf_nocheck_data_alloc from_containers(C1& val, C2& pl) noexcept {
        multi_buf_nocheck_data_alloc ret;
        ret.pval = ret.val_buffer = val.data();
        ret.ppl = ret.pl_buffer = pl.data();
        return ret;
    }

    int64_t* get_value_buffer_ptr() const noexcept {
        return val_buffer;
    }
    int64_t get_value_offset() const noexcept {
        return pval - val_buffer;
    }
    int64_t* alloc_value(size_t n = 1) noexcept {
        auto ret = pval;
        pval += n;
        return ret;
    }

    int64_t* get_payload_buffer_ptr() const noexcept {
        return pl_buffer;
    }
    int64_t get_cur_payload_offset() const noexcept {
        return ppl - pl_buffer - 1;
    }
    int64_t get_cur_payload() const noexcept {
        return *(ppl - 1);
    }
    int64_t* get_payload_ptr() const noexcept {
        return ppl;
    }
    int64_t* alloc_payload() noexcept {
        return ppl++;
    }
    void free_payload(size_t n) noexcept {
        ppl -= n;
    }
};

} // namespace pojobuf::alloc
