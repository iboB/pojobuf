// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "value_type.hpp"
#include <cstdint>
#include <splat/unreachable.h>

namespace pojobuf {

enum class pl_tag : uint32_t {
    null,
    false_,
    true_,
    int32,
    int64,
    real,
    string,
    blob,
    array,
    object,
    sorted_object,
};
inline constexpr uint32_t operator*(pl_tag t) {
    return uint32_t(t);
}

// in buffers we pack array and object elements as `offset << pl_tag_bits | pl_tag`
// since we use uint64_t for elements, this gives us 60 bits for the offset, which is plenty
// see sajson-notes.md for more about this
inline constexpr uint32_t pl_tag_bits = 4;
static_assert(*pl_tag::sorted_object < (1 << pl_tag_bits));

static constexpr value_type get_type_from_pl_tag(pl_tag t) {
    switch (t) {
    case pl_tag::null: return value_type::null;
    case pl_tag::false_: return value_type::false_;
    case pl_tag::true_: return value_type::true_;
    case pl_tag::int32: return value_type::int32;
    case pl_tag::int64: return value_type::int64;
    case pl_tag::real: return value_type::real;
    case pl_tag::string: return value_type::string;
    case pl_tag::blob: return value_type::blob;
    case pl_tag::array: return value_type::array;
    case pl_tag::object: return value_type::object;
    case pl_tag::sorted_object: return value_type::sorted_object;
    }
    SPLAT_UNREACHABLE();
}

} // namespace pojobuf
