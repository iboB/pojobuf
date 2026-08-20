// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <concepts>

namespace pojobuf::bits {

template <std::integral T>
constexpr T divide_round_up(T dividend, T divisor) {
    return (dividend + divisor - 1) / divisor;
}

} // namespace pojobuf::bits
