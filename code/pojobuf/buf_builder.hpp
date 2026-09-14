// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "value_tag.hpp"
#include "valutil.hpp"
#include "bytes_range.hpp"
#include <splat/inline.h>
#include <cassert>

namespace pojobuf {

template <typename DataAlloc, typename ByteAlloc>
class buf_builder {
public:
    using data_alloc_t = DataAlloc;
    using byte_alloc_t = ByteAlloc;

    DataAlloc& adata;
    ByteAlloc& abyte;

    // kinda hacky: we use null to mark the root
    value_tag current_compound_tag = value_tag::null;
    size_t current_compound_base = 0;

    const size_t max_unsorted_obj_records;

    // not sorting objects by default
    explicit buf_builder(DataAlloc& adata, ByteAlloc& abyte, size_t max_unsorted_obj_records = size_t(-1))
        : adata(adata)
        , abyte(abyte)
        , max_unsorted_obj_records(max_unsorted_obj_records)
    {}

    bool current_compound_is_root() const noexcept {
        return current_compound_tag == value_tag::null;
    }
    bool current_compound_is_array() const noexcept {
        return current_compound_tag == value_tag::array;
    }
    bool current_compound_is_object() const noexcept {
        return current_compound_tag == value_tag::object;
    }

    template <value_tag Tag>
    void add_literal_element() {
        *adata.alloc_payload() = int64_t(Tag);
    }

    template <typename Num>
    void add_number_element(Num n) {
        using valutil::make_payload;
        const auto value_offset = adata.get_value_offset();

        static_assert(std::is_integral_v<Num> || std::is_floating_point_v<Num>);
        if constexpr (std::is_integral_v<Num>) {
            *adata.alloc_value() = int64_t(n);
            if constexpr (sizeof(Num) <= 4) {
                *adata.alloc_payload() = make_payload(value_tag::int32, value_offset);
            }
            else {
                *adata.alloc_payload() = make_payload(value_tag::int64, value_offset);
            }
        }
        else {
            *adata.alloc_value() = std::bit_cast<int64_t>(double(n));
            *adata.alloc_payload() = make_payload(value_tag::real, value_offset);
        }
    }

    void add_string_element(const bytes_range& range) {
        const auto value_offset = adata.get_value_offset();
        auto ptr = adata.alloc_value(2);
        ptr[0] = range.begin;
        ptr[1] = range.end;
        *adata.alloc_payload() = valutil::make_payload(value_tag::string, value_offset);
    }

    template <value_tag Tag>
    void open_compound_element() {
        // for compound types make a payload that points to the current compound tag and base
        // so that we can backtrack appropriately
        // it will subsequently be overwritten with the actual computed payload of the compound type
        static_assert(Tag == value_tag::array || Tag == value_tag::object);
        *adata.alloc_payload() = valutil::make_payload(current_compound_tag, current_compound_base);
        current_compound_tag = Tag;
        current_compound_base = adata.get_cur_payload_offset();
    }

    void add_object_key(const bytes_range& range) {
        *adata.alloc_payload() = range.begin;
        *adata.alloc_payload() = range.end;
    }

    bool should_sort_object(size_t records_size) const noexcept {
        return records_size > max_unsorted_obj_records;
    }

    template <value_tag Tag>
    void close_compound_element() {
        if constexpr (DataAlloc::is_single_buf) {
            close_compound_element_single_buf<Tag>();
        }
        else {
            close_compound_element_fwd<Tag>();
        }
    }

    int64_t finalize() noexcept {
        assert(current_compound_tag == value_tag::null);
        return adata.get_cur_payload();
    }

    using piecewise_string_builder = typename ByteAlloc::piecewise_string_builder;
    FORCE_INLINE bytes_range push_string(const char* begin, const char* end) {
        return abyte.push_string(begin, end);
    }
    FORCE_INLINE bytes_range push_internal_string(const char* begin, const char* end) {
        return abyte.push_internal_string(begin, end);
    }
    FORCE_INLINE piecewise_string_builder get_piecewise_string_builder(const char* simple_begin, const char* begin) {
        return abyte.get_piecewise_string_builder(simple_begin, begin);
    }
    FORCE_INLINE bytes_range push_string(const piecewise_string_builder& psb) noexcept {
        return abyte.push_string(psb);
    }

private:

    template <value_tag Tag>
    void close_compound_element_single_buf() {
        using namespace valutil;

        static_assert(Tag == value_tag::array || Tag == value_tag::object);

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
        if constexpr (Tag == value_tag::array) {
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
        else if constexpr (Tag == value_tag::object) {
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
            *pl_end = make_payload(value_tag::sorted_object, value_offset);
        }
        else {
            *pl_end = make_payload(Tag, value_offset);
        }

        adata.tail = pl_end;
    }

    template <value_tag Tag>
    void close_compound_element_fwd() {
        using namespace valutil;
        static_assert(Tag == value_tag::array || Tag == value_tag::object);

        const auto pl_head = adata.get_payload_buffer_ptr() + current_compound_base;
        const auto pl_begin = pl_head + 1;
        const auto pl_end = adata.get_payload_ptr();

        const auto val_offset = adata.get_value_offset();

        const auto length = pl_end - pl_begin;
        const auto length_value = adata.alloc_value(length + 1);
        auto pval = length_value + 1;

        bool is_sorted_object = false;
        if constexpr (Tag == value_tag::array) {
            *length_value = length;
            for (ptrdiff_t i = 0; i < length; ++i) {
                auto elem = pl_begin[i];
                auto offset = get_offset_from_payload(elem);
                pval[i] = make_payload(get_tag_from_payload(elem), offset - val_offset);
            }
        }
        else if constexpr (Tag == value_tag::object) {
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
            *pl_head = make_payload(value_tag::sorted_object, val_offset);
        }
        else {
            *pl_head = make_payload(Tag, val_offset);
        }

        adata.free_payload(length);
    }
};

} // namespace pojobuf
