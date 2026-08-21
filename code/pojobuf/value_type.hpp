// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint>

namespace pojobuf {

struct value_type {
    enum e : uint32_t {
        undefined =       0b001,
        null      =       0b010,
        custom    =       0b100,
        false_ =        0b01000,
        true_  =        0b10000,
        int32  =     0b00100000,
        int64  =     0b01000000,
        real   =     0b10000000,
        string =   0b0100000000,
        blob   =   0b1000000000,
        array  =  0b10000000000,
        object = 0b100000000000,

        // object whose keys are sorted, so we can do binary search on them
        // this is a bit of a special snowflake as it's the only possible value of a type
        // that has two bits set
        // keep this in mind when switching by type
        sorted_object = (object << 1) | object,
    };

    e t;
    constexpr e operator*() const { return t; }

    // masks
    static constexpr uint32_t falsy = undefined | null | false_;
    static constexpr uint32_t boolean = false_ | true_;
    static constexpr uint32_t integer = int32 | int64;
    static constexpr uint32_t number = integer | real;
    static constexpr uint32_t buffer = string | blob;
    static constexpr uint32_t compound = array | object;

    // intentionally implicit
    constexpr value_type(e t) : t(t) {}

    constexpr bool is(uint32_t q) const { return (t & q) != 0; }

    constexpr bool is_undefined() const { return t == undefined; }
    constexpr bool is_null() const { return t == null; }
    constexpr bool is_false() const { return t == false_; }
    constexpr bool is_true() const { return t == true_; }
    constexpr bool is_int32() const { return t == int32; }
    constexpr bool is_int64() const { return t == int64; }
    constexpr bool is_real() const { return t == real; }
    constexpr bool is_string() const { return t == string; }
    constexpr bool is_blob() const { return t == blob; }
    constexpr bool is_array() const { return t == array; }
    constexpr bool is_object() const { return is(object); }
    constexpr bool is_sorted_object() const { return t == object; }

    constexpr bool is_falsy() const { return is(falsy); }
    constexpr bool is_boolean() const { return is(boolean); }
    constexpr bool is_integer() const { return is(integer); }
    constexpr bool is_number() const { return is(number); }
    constexpr bool is_buffer() const { return is(buffer); }
    constexpr bool is_compound() const { return is(compound); }

    constexpr bool operator==(const value_type&) const = default;
    constexpr bool operator!=(const value_type&) const = default;
};

} // namespace pojobuf
