// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint>

namespace pojobuf::json {

// json imposed limits (max integer which can be stored in a double)
static inline constexpr int64_t max_int64 = 9007199254740992ll;
static inline constexpr int64_t min_int64 = -9007199254740992ll;
static inline constexpr uint64_t max_uint64 = 9007199254740992ull;

} // namespace pojobuf::json
