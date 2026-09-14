// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "value_tag.hpp"

namespace pojobuf {

// a thin, cheap-to-construct wrapper around value_tag which adds the category queries
// (is_number, is_buffer, is_compound, ...) that users of pojobuf::value care about.
//
// unlike value_tag, whose values must stay a stable, densely packed 4-bit buffer encoding,
// value_type is free to grow additional queries without any binary compatibility concerns:
// it materializes nothing and is only ever constructed on demand from a value_tag.
struct value_type {
    value_tag t;

    // intentionally implicit
    constexpr value_type(value_tag t) : t(t) {}

    constexpr value_tag operator*() const { return t; }

    constexpr bool is_undefined() const { return t == value_tag::undefined; }
    constexpr bool is_null() const { return t == value_tag::null; }
    constexpr bool is_false() const { return t == value_tag::false_; }
    constexpr bool is_true() const { return t == value_tag::true_; }
    constexpr bool is_int32() const { return t == value_tag::int32; }
    constexpr bool is_int64() const { return t == value_tag::int64; }
    constexpr bool is_real() const { return t == value_tag::real; }
    constexpr bool is_string() const { return t == value_tag::string; }
    constexpr bool is_blob() const { return t == value_tag::blob; }
    constexpr bool is_array() const { return t == value_tag::array; }
    constexpr bool is_object() const { return t == value_tag::object || t == value_tag::sorted_object; }
    constexpr bool is_sorted_object() const { return t == value_tag::sorted_object; }
    constexpr bool is_custom() const { return t == value_tag::custom; }

    constexpr bool is_falsy() const { return t == value_tag::undefined || t == value_tag::null || t == value_tag::false_; }
    constexpr bool is_boolean() const { return t == value_tag::false_ || t == value_tag::true_; }
    constexpr bool is_integer() const { return t == value_tag::int32 || t == value_tag::int64; }
    constexpr bool is_number() const { return is_integer() || t == value_tag::real; }
    constexpr bool is_bytes() const { return t == value_tag::string || t == value_tag::blob; }
    constexpr bool is_compound() const { return t == value_tag::array || is_object(); }

    constexpr bool operator==(const value_type&) const = default;
    constexpr bool operator!=(const value_type&) const = default;
};

} // namespace pojobuf
