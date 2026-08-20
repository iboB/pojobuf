// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <string>
#include <system_error>

namespace pojobuf {

struct parse_error {
    enum class errc : int {
        no_error = 0,
        out_of_memory,
        unexpected_end,
        missing_object_key,
        illegal_codepoint,
        unknown_escape,
        invalid_number,

        // argument errors
        expected_,
        unexpected_,
        invalid_,
    };

    std::string category;

    errc code;
    std::string arg;

    std::string source_id; // for user convenience (the library doesn't touch this)
    int line = 0;
    int column = 0;

    // include parse_error.create for this
    static parse_error create(
        std::string cat,
        errc code, std::string arg,
        const char* text, const char* error_pos
    );
};

} // namespace pojobuf
