// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "parse_error.hpp"
#include "bits/get_text_location.hpp"

namespace pojobuf {

inline parse_error parse_error::create(
    std::string cat,
    errc code, std::string arg,
    const char* text, const char* error_pos
) {
    parse_error ret;
    ret.category = std::move(cat);
    ret.code = code;
    ret.arg = std::move(arg);
    std::tie(ret.line, ret.column) = bits::get_text_location(text, error_pos);
    return ret;
}

} // namespace pojobuf
