// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint>

namespace pojobuf {

// undefined is never written to a buffer: it only appears as the tag of a default-constructed
// (or "not found") value. It's placed first so that it groups contiguously with null and false_
// for the falsy check.
//
// this is also the tag exposed as pojobuf::value_type: keep it stable, don't reorder existing
// values, and only ever append new ones after custom. Buffers built by one version of the
// library are only guaranteed to be readable by another if this order is preserved. A change to
// this order is a new major version and should come with a conversion utility for old buffers.
enum class pl_tag : uint8_t {
    undefined,
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

    custom // keep last
};
constexpr uint32_t operator*(pl_tag t) {
    return uint32_t(t);
}

// in buffers we pack array and object elements as `offset << pl_tag_bits | pl_tag`
// since we use uint64_t for elements, this gives us 60 bits for the offset, which is plenty
// see sajson-notes.md for more about this
inline constexpr uint32_t pl_tag_bits = 4;
static_assert(*pl_tag::custom < (1 << pl_tag_bits));

} // namespace pojobuf
