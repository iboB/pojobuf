// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "pl_tag.hpp"

namespace pojobuf {

// a thin, cheap-to-construct wrapper around pl_tag which adds the category queries
// (is_number, is_buffer, is_compound, ...) that users of pojobuf::value care about.
//
// unlike pl_tag, whose values must stay a stable, densely packed 4-bit buffer encoding,
// value_type is free to grow additional queries without any binary compatibility concerns:
// it materializes nothing and is only ever constructed on demand from a pl_tag.
struct value_type {
    pl_tag t;

    // intentionally implicit
    constexpr value_type(pl_tag t) : t(t) {}

    constexpr pl_tag operator*() const { return t; }

    constexpr bool is_undefined() const { return t == pl_tag::undefined; }
    constexpr bool is_null() const { return t == pl_tag::null; }
    constexpr bool is_false() const { return t == pl_tag::false_; }
    constexpr bool is_true() const { return t == pl_tag::true_; }
    constexpr bool is_int32() const { return t == pl_tag::int32; }
    constexpr bool is_int64() const { return t == pl_tag::int64; }
    constexpr bool is_real() const { return t == pl_tag::real; }
    constexpr bool is_string() const { return t == pl_tag::string; }
    constexpr bool is_blob() const { return t == pl_tag::blob; }
    constexpr bool is_array() const { return t == pl_tag::array; }
    constexpr bool is_object() const { return t == pl_tag::object || t == pl_tag::sorted_object; }
    constexpr bool is_sorted_object() const { return t == pl_tag::sorted_object; }
    constexpr bool is_custom() const { return t == pl_tag::custom; }

    constexpr bool is_falsy() const { return t == pl_tag::undefined || t == pl_tag::null || t == pl_tag::false_; }
    constexpr bool is_boolean() const { return t == pl_tag::false_ || t == pl_tag::true_; }
    constexpr bool is_integer() const { return t == pl_tag::int32 || t == pl_tag::int64; }
    constexpr bool is_number() const { return is_integer() || t == pl_tag::real; }
    constexpr bool is_buffer() const { return t == pl_tag::string || t == pl_tag::blob; }
    constexpr bool is_compound() const { return t == pl_tag::array || is_object(); }

    constexpr bool operator==(const value_type&) const = default;
    constexpr bool operator!=(const value_type&) const = default;
};

} // namespace pojobuf
