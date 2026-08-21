// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "parse_error.hpp"
#include <string_view>

namespace pojobuf {

constexpr std::string_view to_str(parse_error::errc code) {
    switch (code) {
    case parse_error::errc::no_error: return "no_error";
    case parse_error::errc::out_of_memory: return "out of memory";
    case parse_error::errc::unexpected_end: return "unexpected end";
    case parse_error::errc::missing_object_key: return "missing object key";
    case parse_error::errc::illegal_codepoint: return "illegal codepoint";
    case parse_error::errc::unknown_escape: return "unknown escape";
    case parse_error::errc::invalid_number: return "invalid number";
    case parse_error::errc::expected_: return "expected";
    case parse_error::errc::unexpected_: return "unexpected";
    case parse_error::errc::invalid_: return "invalid";
    }
    return "<unknown>";
}

inline std::string to_str(const parse_error& e) {
    std::string str = e.category + ": ";

    if (e.source_id.empty()) {
        str += "<source>:";
    }
    else {
        str += e.source_id + ":";
    }

    str += std::to_string(e.line) + ":" + std::to_string(e.column) + ": ";
    str += to_str(e.code);

    if (!e.arg.empty()) {
        str += " '" + e.arg + "'";
    }
    return str;
}

} // namespace pojobuf
