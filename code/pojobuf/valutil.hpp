// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "pl_tag.hpp"
#include <cstdint>
#include <cstring>
#include <string_view>

namespace pojobuf::valutil {

constexpr pl_tag get_tag_from_payload(int64_t payload) {
    return pl_tag(payload & ((1 << pl_tag_bits) - 1));
}
constexpr int64_t get_offset_from_payload(int64_t payload) {
    return payload >> pl_tag_bits;
}
constexpr int64_t make_payload(pl_tag t, int64_t offset) {
    return (offset << pl_tag_bits) | int64_t(t);
}

struct object_elem {
    int64_t key_start;
    int64_t key_end;
    int64_t value_payload;
    static constexpr size_t num_fields = 3;
};
static_assert(sizeof(object_elem) == object_elem::num_fields * sizeof(int64_t));

// only used in sorted objects
struct object_key_cmp {
    const char* byte_ptr;

    // instead of doing a lexicographical sort, first sort by length
    // thus we won't touch the text memory for most comparisons
    bool operator()(const object_elem& a, const object_elem& b) const noexcept {
        const auto a_len = a.key_end - a.key_start;
        const auto b_len = b.key_end - b.key_start;
        if (a_len < b_len) return true;
        if (a_len > b_len) return false;
        return std::memcmp(byte_ptr + a.key_start, byte_ptr + b.key_start, a_len) < 0;
    }
    bool operator()(const object_elem& a, std::string_view b) const noexcept {
        const auto a_len = a.key_end - a.key_start;
        const auto b_len = int64_t(b.size());
        if (a_len < b_len) return true;
        if (a_len > b_len) return false;
        return std::memcmp(byte_ptr + a.key_start, b.data(), a_len) < 0;
    }
};

} // namespace pojobuf::valutil
