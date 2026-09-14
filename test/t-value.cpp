// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <pojobuf/value.hpp>
#include <pojobuf/value.array_elements.hpp>
#include <pojobuf/value.object_elements.hpp>

#include <doctest/doctest.h>

#include <pojobuf/document_parse.hpp>
#include <pojobuf/json/parser.hpp>

TEST_CASE("empty") {
    pojobuf::value val;
    CHECK(val.tag() == pojobuf::value_tag::undefined);
    CHECK_FALSE(val.data_ptr());
    CHECK_FALSE(val.byte_ptr());
    CHECK(val.type().is_undefined());
}

TEST_CASE("value getters") {
    std::string_view json = R"([null, true, false, 42, 5000000006, 3.14, "horse"])";
    auto doc = pojobuf::document_parse<pojobuf::json::parser_charconv_num>(json);
    REQUIRE(doc);
    auto root = doc->root();
    REQUIRE(root.type().is_array());

    {
        auto val = root.array_element_at(0);
        CHECK(val.type().is_null());
    }

    {
        auto val = root.array_element_at(1);
        CHECK(val.type().is_true());
        CHECK(val.boolean_value());
    }

    {
        auto val = root.array_element_at(2);
        CHECK(val.type().is_false());
        CHECK_FALSE(val.boolean_value());
    }

    {
        auto val = root.array_element_at(3);
        CHECK(val.type().is_int32());
        CHECK(val.int32_value() == 42);
        CHECK(val.int64_value() == 42);
        CHECK(val.integer_value() == 42);
        CHECK(val.integer_value_safe() == 42);
        CHECK(val.real_value_safe() == 42.);
        CHECK(val.f64_value() == 42.);
        CHECK(val.f32_value() == 42.f);
    }

    {
        auto val = root.array_element_at(4);
        CHECK(val.type().is_int64());
        CHECK(val.int64_value() == 5'000'000'006ll);
        CHECK(val.integer_value() == 5'000'000'006ll);
        CHECK(val.integer_value_safe() == 5'000'000'006ll);
        CHECK(val.real_value_safe() == 5e9 + 6.);
        CHECK(val.f64_value() == 5'000'000'006.);
    }

    {
        auto val = root.array_element_at(5);
        CHECK(val.type().is_real());
        CHECK(val.real_value() == 3.14);
        CHECK(val.real_value_safe() == 3.14);
        CHECK(val.f64_value() == 3.14);
        CHECK(val.f32_value() == 3.14f);
        CHECK(val.integer_value_safe() == 3);
    }

    {
        auto val = root.array_element_at(6);
        CHECK(val.type().is_string());
        CHECK(val.string_length() == 5);
        CHECK(val.blob_size() == 5);
        CHECK(val.bytes_size() == 5);

        static constexpr std::string_view expected = "horse";
        CHECK(val.string_value() == expected);
        auto check_bytes = [](auto span) {
            CHECK(span.size() == 5);
            for (int i = 0; i < 5; ++i) {
                CHECK(char(span[i]) == expected[i]);
            }
        };
        check_bytes(val.bytes_value());
        check_bytes(val.blob_value());
    }
}

TEST_CASE("array") {
    std::string_view json = R"([0, 1, 2, 3, 4])";
    auto doc = pojobuf::document_parse<pojobuf::json::parser_charconv_num>(json);
    REQUIRE(doc);
    auto root = doc->root();
    REQUIRE(root.type().is_array());

    CHECK(root.array_length() == 5);
    CHECK(root.compound_length() == 5);

    for (int i = 0; i < 5; ++i) {
        {
            auto val = root.array_element_at(i);
            CHECK(val.type().is_int32());
            CHECK(val.int32_value() == i);
        }
        {
            auto val = root.array_element_at_safe(i);
            CHECK(val.type().is_int32());
            CHECK(val.int32_value() == i);
        }
    }

    {
        auto val = root.array_element_at_safe(432);
        CHECK(val.type().is_undefined());
    }

    int i = 0;
    for (auto val : root.array_elements()) {
        CHECK(val.type().is_int32());
        CHECK(val.int32_value() == i);
        ++i;
    }

    auto ia = root.array_elements().begin();
    auto ib = root.array_elements().begin();
    CHECK(ia == ib);
    ++ia;
    CHECK(ia != ib);
    CHECK(ia > ib);
    CHECK(ib < ia);
    ++ib;
    CHECK(ia == ib);

    {
        auto val = *ia;
        CHECK(val.type().is_int32());
        CHECK(val.int32_value() == 1);
    }

    ++ia;
    ++ia;
    ++ia;
    ++ia;

    CHECK(ia == root.array_elements().end());
}

TEST_CASE("object") {
    std::string_view json = R"({
        "a": 0,
        "b": 1,
        "c": 2,
        "d": 3
    })";

    auto do_test = [](const pojobuf::value& root) {
        CHECK(root.object_length() == 4);
        CHECK(root.compound_length() == 4);

        char k;
        std::string_view ekey(&k, 1);
        for (int i = 0; i < 4; ++i) {
            k = char('a' + i);
            CHECK(root.object_key_at(i) == ekey);
            {
                auto val = root.object_value_at(i);
                CHECK(val.int32_value() == i);
            }
            {
                auto val = root.object_value_at_safe(i);
                CHECK(val.int32_value() == i);
            }
            {
                auto [key, val] = root.object_element_at(i);
                CHECK(key == ekey);
                CHECK(val.int32_value() == i);
            }
            {
                auto [key, val] = root.object_element_at_safe(i);
                CHECK(key == ekey);
                CHECK(val.int32_value() == i);
            }
            {
                auto f = root.find_object_key(ekey);
                CHECK(f == i);
            }
            {
                auto val = root.object_value_at_key(ekey);
                CHECK(val.int32_value() == i);
            }
            {
                auto val = root.object_value_at_key_safe(ekey);
                CHECK(val.int32_value() == i);
            }
        }

        {
            auto val = root.object_value_at_safe(23);
            CHECK(val.type().is_undefined());
        }
        {
            auto [key, val] = root.object_element_at_safe(41);
            CHECK(key.empty());
            CHECK(val.type().is_undefined());
        }
        {
            auto f = root.find_object_key("z");
            CHECK(f == 4);
            f = root.find_object_key("ab");
            CHECK(f == 4);
        }
        {
            auto val = root.object_value_at_key_safe("zz");
            CHECK(val.type().is_undefined());
        }

        int i = 0;
        for (auto [key, val] : root.object_elements()) {
            k = char('a' + i);
            CHECK(key == ekey);
            CHECK(val.int32_value() == i);
            ++i;
        }
    };

    {
        auto doc = pojobuf::document_parse<pojobuf::json::parser_charconv_num>(json);
        REQUIRE(doc);
        auto root = doc->root();
        REQUIRE(root.type().is_object());
        REQUIRE_FALSE(root.type().is_sorted_object());
        do_test(root);
    }

    {
        auto doc = pojobuf::document_parse<pojobuf::json::parser_charconv_num>(json, 0);
        REQUIRE(doc);
        auto root = doc->root();
        REQUIRE(root.type().is_sorted_object());
        do_test(root);
    }
}
