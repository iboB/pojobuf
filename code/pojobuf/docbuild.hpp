// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "pl_tag.hpp"
#include "docstore.hpp"
#include "buffer_range.hpp"
#include "bits/imath.hpp"
#include <splat/inline.h>
#include <cassert>

namespace pojobuf::docbuild {

using namespace docstore;

constexpr int64_t make_payload(pl_tag t, int64_t value) {
    return (value << pl_tag_bits) | int64_t(t);
}

template <typename DataAlloc, typename ByteAlloc>
class buf_builder {
public:
    using data_alloc_t = DataAlloc;
    using byte_alloc_t = ByteAlloc;

    DataAlloc& adata;
    ByteAlloc& abyte;

    // kinda hacky: we use null to mark the root
    pl_tag current_compound_tag = pl_tag::null;
    size_t current_compound_base = 0;

    const size_t max_unsorted_obj_records;

    // not sorting objects by default
    explicit buf_builder(DataAlloc& adata, ByteAlloc& abyte, size_t max_unsorted_obj_records = size_t(-1))
        : adata(adata)
        , abyte(abyte)
        , max_unsorted_obj_records(max_unsorted_obj_records)
    {}

    bool current_compound_is_root() const noexcept {
        return current_compound_tag == pl_tag::null;
    }
    bool current_compound_is_array() const noexcept {
        return current_compound_tag == pl_tag::array;
    }
    bool current_compound_is_object() const noexcept {
        return current_compound_tag == pl_tag::object;
    }

    template <pl_tag Tag>
    void add_literal_element() {
        *adata.alloc_payload() = int64_t(Tag);
    }

    template <typename Num>
    void add_number_element(Num n) {
        const auto value_offset = adata.get_value_offset();

        static_assert(std::is_integral_v<Num> || std::is_floating_point_v<Num>);
        if constexpr (std::is_integral_v<Num>) {
            *adata.alloc_value() = int64_t(n);
            if constexpr (sizeof(Num) <= 4) {
                *adata.alloc_payload() = make_payload(pl_tag::int32, value_offset);
            }
            else {
                *adata.alloc_payload() = make_payload(pl_tag::int64, value_offset);
            }
        }
        else {
            *adata.alloc_value() = std::bit_cast<int64_t>(double(n));
            *adata.alloc_payload() = make_payload(pl_tag::real, value_offset);
        }
    }

    void add_string_element(const buffer_range& range) {
        const auto value_offset = adata.get_value_offset();
        auto ptr = adata.alloc_value(2);
        ptr[0] = range.begin;
        ptr[1] = range.end;
        *adata.alloc_payload() = make_payload(pl_tag::string, value_offset);
    }

    template <pl_tag Tag>
    void open_compound_element() {
        // for compound types make a payload that points to the current compound tag and base
        // so that we can backtrack appropriately
        // it will subsequently be overwritten with the actual computed payload of the compound type
        static_assert(Tag == pl_tag::array || Tag == pl_tag::object);
        *adata.alloc_payload() = make_payload(current_compound_tag, current_compound_base);
        current_compound_tag = Tag;
        current_compound_base = adata.get_cur_payload_offset();
    }

    void add_object_key(const buffer_range& range) {
        *adata.alloc_payload() = range.begin;
        *adata.alloc_payload() = range.end;
    }

    bool should_sort_object(size_t records_size) const noexcept {
        return records_size > max_unsorted_obj_records;
    }

    template <pl_tag Tag>
    void close_compound_element() {
        if constexpr (DataAlloc::is_single_buf) {
            close_compound_element_single_buf<Tag>();
        }
        else {
            close_compound_element_fwd<Tag>();
        }
    }

    int64_t finalize() noexcept {
        assert(current_compound_tag == pl_tag::null);
        return adata.get_cur_payload();
    }


    using piecewise_string_builder = typename ByteAlloc::piecewise_string_builder;
    FORCE_INLINE buffer_range push_string(const char* begin, const char* end) {
        return abyte.push_string(begin, end);
    }
    FORCE_INLINE buffer_range push_internal_string(const char* begin, const char* end) {
        return abyte.push_internal_string(begin, end);
    }
    FORCE_INLINE piecewise_string_builder get_piecewise_string_builder(const char* simple_begin, const char* begin) {
        return abyte.get_piecewise_string_builder(simple_begin, begin);
    }
    FORCE_INLINE buffer_range push_string(const piecewise_string_builder& psb) noexcept {
        return abyte.push_string(psb);
    }

private:

    template <pl_tag Tag>
    void close_compound_element_single_buf() {
        static_assert(Tag == pl_tag::array || Tag == pl_tag::object);

        const auto pl_begin = adata.tail;
        const auto pl_end = adata.get_value_buffer_ptr() + current_compound_base;
        auto& head_ptr = adata.head;
        const auto value_offset = adata.get_value_offset();
        auto length_value = adata.alloc_value();

        auto transfer_elem = [&value_offset](int64_t elem) FORCE_INLINE_LAMBDA {
            const auto abs_offset = get_offset_from_payload(elem);
            const auto rel_offset = abs_offset - value_offset;
            const auto tag = get_tag_from_payload(elem);
            return make_payload(tag, rel_offset);
        };

        const auto length = pl_end - pl_begin;
        bool is_sorted_object = false;
        if constexpr (Tag == pl_tag::array) {
            *length_value = length;
            if (head_ptr + length > pl_begin) [[unlikely]] {
                // overlap: reverse and move forward
                std::reverse(pl_begin, pl_end);
                for (ptrdiff_t i = 0; i < length; ++i) {
                    auto elem = pl_begin[i];
                    head_ptr[i] = transfer_elem(elem);
                }
            }
            else {
                // no overlap: we can move while reversing
                for (ptrdiff_t i = 0; i < length; ++i) {
                    auto elem = pl_end[-i - 1];
                    head_ptr[i] = transfer_elem(elem);
                }
            }
        }
        else if constexpr (Tag == pl_tag::object) {
            assert(length % object_elem::num_fields == 0);
            const auto records_size = length / ptrdiff_t(object_elem::num_fields);
            *length_value = records_size;

            const auto data_records_begin = reinterpret_cast<object_elem*>(head_ptr);
            auto data_records_end = data_records_begin;

            for (ptrdiff_t i = 0; i < length; i += object_elem::num_fields) {
                static_assert(object_elem::num_fields == 3);
                *data_records_end = object_elem{
                    .key_start = pl_end[-i - 1],
                    .key_end   = pl_end[-i - 2],
                    .value_payload = transfer_elem(pl_end[-i - 3]),
                };
                ++data_records_end;
            }

            if (should_sort_object(records_size)) [[unlikely]] {
                // here we sort in-place after we've moved the records
                // however since we have a free scratch buffer, we can do smarter things
                // we can create abbreviated keys and do a prefix sort which should significantly improve sorting perf
                // since this is expected to be a rare case, we leave this optimization for the future
                std::sort(data_records_begin, data_records_end, object_key_cmp{abyte.get_byte_ptr()});
                is_sorted_object = true;
            }
        }

        head_ptr += length;
        current_compound_tag = get_tag_from_payload(*pl_end);
        current_compound_base = get_offset_from_payload(*pl_end);

        if (is_sorted_object) [[unlikely]] {
            *pl_end = make_payload(pl_tag::sorted_object, value_offset);
        }
        else {
            *pl_end = make_payload(Tag, value_offset);
        }

        adata.tail = pl_end;
    }

    template <pl_tag Tag>
    void close_compound_element_fwd() {
        static_assert(Tag == pl_tag::array || Tag == pl_tag::object);

        const auto pl_head = adata.get_payload_buffer_ptr() + current_compound_base;
        const auto pl_begin = pl_head + 1;
        const auto pl_end = adata.get_payload_ptr();

        const auto val_offset = adata.get_value_offset();
        auto length_value = adata.alloc_value();

        const auto length = pl_end - pl_begin;
        auto pval = adata.alloc_value(length);

        bool is_sorted_object = false;
        if constexpr (Tag == pl_tag::array) {
            *length_value = length;
            for (ptrdiff_t i = 0; i < length; ++i) {
                auto elem = pl_begin[i];
                auto offset = get_offset_from_payload(elem);
                pval[i] = make_payload(get_tag_from_payload(elem), offset - val_offset);
            }
        }
        else if constexpr (Tag == pl_tag::object) {
            assert(length % object_elem::num_fields == 0);
            auto records_begin = reinterpret_cast<object_elem*>(pl_begin);
            auto records_size = length / ptrdiff_t(object_elem::num_fields);
            *length_value = records_size;

            auto data_records_begin = reinterpret_cast<object_elem*>(pval);

            if (should_sort_object(records_size)) [[unlikely]] {
                // here we sort and move
                // however since we have a free scratch buffer, we can do smarter things
                // we can create abbreviated keys and do a prefix sort which should significantly improve sorting perf
                // since this is expected to be a rare case, we leave this optimization for the future
                std::sort(records_begin, records_begin + records_size, object_key_cmp{abyte.get_byte_ptr()});
                is_sorted_object = true;
            }

            for (ptrdiff_t i = 0; i < records_size; ++i) {
                auto& r = records_begin[i];
                auto offset = get_offset_from_payload(r.value_payload);
                data_records_begin[i] = object_elem{
                    r.key_start,
                    r.key_end,
                    make_payload(get_tag_from_payload(r.value_payload), offset - val_offset),
                };
            }
        }

        current_compound_tag = get_tag_from_payload(*pl_head);
        current_compound_base = get_offset_from_payload(*pl_head);

        if (is_sorted_object) [[unlikely]] {
            *pl_head = make_payload(pl_tag::sorted_object, val_offset);
        }
        else {
            *pl_head = make_payload(Tag, val_offset);
        }

        adata.free_payload(length);
    }
};

class mutable_source_byte_alloc {
    char* m_text;
public:
    explicit mutable_source_byte_alloc(char* text) noexcept
        : m_text(text)
    {}

    const char* get_byte_ptr() const noexcept {
        return m_text;
    }

    buffer_range push_internal_string(const char* begin, const char* end) noexcept {
        assert(begin > m_text);
        return buffer_range{begin - m_text, end - m_text};
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
    buffer_range push_string(const piecewise_string_builder& psb) noexcept {
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

    buffer_range push_string(const char* begin, const char* end) {
        const int64_t begin_offset = m_data_alloc.get_value_offset() * sizeof(int64_t);
        const int64_t length = end - begin;

        if (length == 0) [[unlikely]] {
            return buffer_range{begin_offset, begin_offset};
        }

        const auto value_length = bits::divide_round_up(length, int64_t(sizeof(int64_t)));
        auto ptr = m_data_alloc.alloc_value(value_length);
        std::memcpy(ptr, begin, length);
        return buffer_range{begin_offset, begin_offset + length};
    }

    buffer_range push_internal_string(const char* begin, const char* end) {
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
    buffer_range push_string(const piecewise_string_builder& psb) noexcept {
        return buffer_range{psb.begin_offset, psb.begin_offset + psb.length};
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


} // namespace pojobuf::docbuild
