// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <pojobuf/value.array_elements.hpp>
#include <pojobuf/value.object_elements.hpp>
#include <pojobuf/document_parse.hpp>
#include <pojobuf/json/parser.hpp>
#include <iostream>

void indent(int depth) {
    assert(depth >= 0);
    while(depth--) std::cout << "    ";
};

void rdump(int depth, const pojobuf::value& val) {
    using enum pojobuf::value_tag;
    switch (*val.type()) {
    case array: {
        std::cout << "<array of " << val.array_length() << " elements> [\n";
        for (auto elem : val.array_elements()) {
            indent(depth + 1);
            rdump(depth + 1, elem);
            std::cout << "\n";
        }
        indent(depth);
        std::cout << "]";
        break;
    }
    case object:
    case sorted_object: {
        std::cout << "<object with " << val.array_length() << " values> {\n";
        for (auto [key, value] : val.object_elements()) {
            indent(depth + 1);
            std::cout << key << " = ";
            rdump(depth + 1, value);
            std::cout << "\n";
        }
        indent(depth);
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
    rdump(0, val);
    std::cout << "\n";
}

int main() {
    constexpr std::string_view json = R"json([
        {
            "character": "John Snow",
            "HP": 20,
            "MP": 10,
            "skills": [
                {"skill": "Sword", "MP": 1},
                {"skill": "Immortality", "MP": 10}
            ],
            "familiar": {"familiar": "Ghost", "HP": 5, "skill": {"skill": "Bite", "MP": 1}}
        },
        {
            "character": "Hodor",
            "HP": 40,
            "MP": 0,
            "skills": [
                {"skill": "Hodor", "MP": 0}
            ]
        }
    ])json";

    auto doc = pojobuf::document_parse<pojobuf::json::parser_charconv_num>(json);
    auto root = doc->root();

    // get concrete values
    auto john = root.array_element_at(0);
    auto skills = john.object_value_at_key("skills");
    auto s0 = skills.array_element_at(0);

    std::cout << "First skill of John Snow: "
        << s0.object_value_at_key("skill").string_value()
        << ", MP cost = " << s0.object_value_at_key("MP").int32_value()
        << "\n";

    // walk entire document
    dump(root);
}
