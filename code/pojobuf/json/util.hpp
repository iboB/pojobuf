// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../bits/imath.hpp"
#include <cstddef>
#include <string_view>

namespace pojobuf::json {

inline size_t get_buffer_size_for_json(size_t json_size) {
    if (json_size == 0) return 0;
    return json_size + 1;
}
inline size_t get_buffer_size_for_json(std::string_view json) {
    return get_buffer_size_for_json(json.size());
}

inline size_t get_scratch_buffer_size_for_json(size_t json_size) {
    return bits::divide_round_up(json_size, size_t(2));
}
inline size_t get_scratch_buffer_size_for_json(std::string_view json) {
    return get_scratch_buffer_size_for_json(json.size());
}

} // namespace pojobuf::json
