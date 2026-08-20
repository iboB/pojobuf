// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <pojobuf/value.hpp>
#include <pojobuf/docbuild.hpp>
#include <pojobuf/json/parser.hpp>
#include <pojobuf/json/util.hpp>
#include <iostream>
#include <vector>

void rdump(pojobuf::value val) {
    using enum pojobuf::value_type::e;
    switch (*val.type()) {
    case array: {
        std::cout << "[";
        for (size_t i = 0; i < val.get_compound_length(); ++i) {
            rdump(val.get_array_element(i));
            if (i < val.get_compound_length() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]";
        break;
    }
    case object:
    case sorted_object: {
        std::cout << "{";
        for (size_t i = 0; i < val.get_compound_length(); ++i) {
            auto key = val.get_object_key(i);
            std::cout << key << ": ";
            rdump(val.get_object_value(i));
            if (i < val.get_compound_length() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "}";
        break;
    }
    case string:
        std::cout << "\"" << val.get_string_value() << "\"";
        break;
    case int32:
        std::cout << val.get_int32_value();
        break;
    case real:
        std::cout << val.get_real_value();
        break;
    case true_:
        std::cout << "true";
        break;
    case false_:
        std::cout << "false";
        break;
    case null:
        std::cout << "null";
        break;
    default:
        std::cout << "???";
        break;
    }
}

void dump(pojobuf::value val) {
    rdump(val);
    std::cout << "\n";
}

int main() {
    char json[] = R"({"ar": [2.3, -5], "val": 5, "b": false, "str": "hello world"})";

    std::vector<int64_t> buffer(pojobuf::json::get_buffer_size_for_json(sizeof(json)));
    [[maybe_unused]] std::vector<int64_t> scratch_buf(pojobuf::json::get_scratch_buffer_size_for_json(json));

    auto data_alloc = pojobuf::docbuild::single_buf_nocheck_data_alloc::from_container(buffer);
    //auto data_alloc = pojobuf::docbuild::multi_buf_nocheck_data_alloc::from_containers(buffer, scratch_buf);

    //pojobuf::docbuild::mutable_source_byte_alloc byte_alloc(json);
    pojobuf::docbuild::valuebuf_byte_alloc byte_alloc(data_alloc);

    pojobuf::docbuild::buf_builder builder(data_alloc, byte_alloc);

    pojobuf::json::parse(json, builder);

    auto cur_payload = builder.finalize();
    pojobuf::value root(cur_payload, data_alloc.get_value_buffer_ptr(), byte_alloc.get_byte_ptr());

    dump(root);

    return 0;
}
