// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <itlib/pod_vector.hpp>

namespace pojobuf::bits {

struct noinit_pod_allocator : public itlib::impl::pod_allocator {
    static constexpr bool zero_fill_new() { return false; }
};
using pod_vector = itlib::pod_vector<int64_t, noinit_pod_allocator>;

} // namespace pojobuf::bits
