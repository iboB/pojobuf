// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <utility>

namespace pojobuf::bits {

inline std::pair<int, int> get_text_location(const char* text, const char* pos) noexcept {
    int line = 1, column = 1;
    for (const char* c = text; c < pos; ++c) {
        if (*c == '\n') {
            ++line;
            column = 1;
        }
        else {
            // TODO: utf-8 aware column counting
            ++column;
        }
    }
    return {line, column};
}

} // namespace pojobuf::bits
