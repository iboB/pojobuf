// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

namespace pojobuf {

enum class invalid_value_strategy {
    no_check, // no check (resulting in a potentially invalid result)
    skip, // skip value entirely
    null, // write null
};

} // namespace pojobuf
