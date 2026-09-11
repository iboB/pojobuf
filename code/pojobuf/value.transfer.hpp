// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "value.hpp"

namespace pojobuf {

// a generic value transfer function
// note that if you know what you're doing, much smarter transfers are possible
// (to be implemented in transfer_ex)
template <typename Builder>
void value_transfer(Builder& b, const value& v) {
    using enum pl_tag;
    switch (v.tag()) {
    case null:
        b.template add_literal_element<null>();
    break;
    case false_:
        b.template add_literal_element<false_>();
    break;
    case true_:
        b.template add_literal_element<true_>();
    break;
    case int32:
        b.add_number_element(v.int32_value());
    break;
    case int64:
        b.add_number_element(v.int64_value());
    break;
    case real:
        b.add_number_element(v.real_value());
    break;
    case string: {
        auto range = b.push_string(v.string_value());
        b.add_string_element(range);
    }
    break;
    case array: {
        b.template open_compound_element<array>();
        b.template close_compound_element<array>();
    }
    break;
    case object: {
        b.template open_compound_element<object>();
        b.template close_compound_element<object>();
    }
    break;
    case sorted_object: {
        b.template open_compound_element<sorted_object>();
        b.template close_compound_element<sorted_object>();
    }
    break;
    default:
        assert(false); // not supported yet
    }
}

} // namespace pojobuf
