// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <pojobuf/value.hpp>
#include <pojobuf/buf_builder.hpp>
#include <pojobuf/alloc.hpp>
#include <pojobuf/json/parser.hpp>
#include <pojobuf/json/util.hpp>
#include <pojobuf/json/writer.hpp>
#include <pojobuf/container_string_sink.hpp>
#include <pojobuf/ostream_string_sink.hpp>
#include <pojobuf/value.transfer.hpp>
#include <pojobuf/value.array_elements.hpp>
#include <iostream>
#include <vector>

void rdump(pojobuf::value val) {
    using enum pojobuf::value_tag;
    switch (*val.type()) {
    case array: {
        std::cout << "[";
        for (size_t i = 0; i < val.compound_length(); ++i) {
            rdump(val.array_element_at(i));
            if (i < val.compound_length() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]";
        break;
    }
    case object:
    case sorted_object: {
        std::cout << "{";
        for (size_t i = 0; i < val.compound_length(); ++i) {
            auto [key, value] = val.object_element_at(i);
            std::cout << key << ": ";
            rdump(value);
            if (i < val.compound_length() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "}";
        break;
    }
    case string:
        std::cout << "\"" << val.string_value() << "\"";
        break;
    case int32:
        std::cout << val.int32_value();
        break;
    case real:
        std::cout << val.real_value();
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
    char json[] = R"({"ar": [2.3, -5, [1]], "val": 5, "b": false, "str": "hello world"})";

    std::vector<int64_t> buffer(pojobuf::json::get_buffer_size_for_json(sizeof(json)));
    [[maybe_unused]] std::vector<int64_t> scratch_buf(pojobuf::json::get_scratch_buffer_size_for_json(json));

    auto data_alloc = pojobuf::alloc::single_buf_nocheck_data_alloc::from_container(buffer);
    ////auto data_alloc = pojobuf::multi_buf_nocheck_data_alloc::from_containers(buffer, scratch_buf);

    ////pojobuf::mutable_source_byte_alloc byte_alloc(json);
    pojobuf::alloc::valuebuf_byte_alloc byte_alloc(data_alloc);

    pojobuf::buf_builder builder(data_alloc, byte_alloc);

    pojobuf::json::parser_charconv_num::parse(json, builder);

    auto cur_payload = builder.finalize();
    pojobuf::value root(cur_payload, data_alloc.get_value_buffer_ptr(), byte_alloc.get_byte_ptr());

    dump(root);

    //pojobuf::ostream_string_sink sink(std::cout);
    //pojobuf::json::writer w(sink, true);

    //pojobuf::json::parser_charconv_num::parse(json, w);

    return 0;
}
