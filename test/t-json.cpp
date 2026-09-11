// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <pojobuf/value.hpp>
#include <pojobuf/docbuild.hpp>
#include <pojobuf/document.hpp>
#include <pojobuf/document.parse.hpp>
#include <pojobuf/json/parser.hpp>
#include <pojobuf/json/util.hpp>
#include <pojobuf/container_string_sink.hpp>
#include <pojobuf/json/writer.hpp>
#include <pojobuf/bits/pod_vector.hpp>

#include <doctest/doctest.h>

#include <pojobuf/dev/read_file.hpp>

#include <nlohmann/json.hpp> // oracle
#include <json-test-data.h>

using nljson = nlohmann::ordered_json;

void rcmp(pojobuf::value val, const nljson& oracle) {
    using enum pojobuf::pl_tag;
    switch (*val.type()) {
    case array: {
        CHECK(oracle.is_array());
        CHECK(val.get_compound_length() == oracle.size());
        for (size_t i = 0; i < val.get_compound_length(); ++i) {
            rcmp(val.get_array_element(i), oracle[i]);
        }
        break;
    }
    case object: {
        // ordered json must be in the same order
        CHECK(oracle.is_object());
        CHECK(val.get_compound_length() == oracle.size());
        size_t i = 0;
        for (auto& [k, v] : oracle.items()) {
            CHECK(val.get_object_key(i) == k);
            rcmp(val.get_object_value(i), v);
            ++i;
        }
        break;
    }
    case sorted_object: {
        // compare ignoring order, since sorted_object is sorted by key
        CHECK(oracle.is_object());
        CHECK(val.get_compound_length() == oracle.size());
        for (size_t i = 0; i < val.get_compound_length(); ++i) {
            auto key = val.get_object_key(i);
            CHECK(oracle.contains(key));
            rcmp(val.get_object_value(i), oracle[key]);
        }
        break;
    }
    case string:
        CHECK(oracle.is_string());
        CHECK(val.get_string_value() == oracle.get<std::string_view>());
        break;
    case int32:
        CHECK(oracle.is_number_integer());
        CHECK(val.get_int32_value() == oracle.get<int32_t>());
        break;
    case int64:
        CHECK(oracle.is_number_integer());
        CHECK(val.get_int64_value() == oracle.get<int64_t>());
        break;
    case real:
        CHECK(oracle.is_number());
        if (oracle.is_number_integer()) {
            auto i64 = oracle.get<int64_t>();
            CHECK((i64 > std::numeric_limits<int32_t>::max() || i64 < std::numeric_limits<int32_t>::min()));
        }
        CHECK(val.get_real_value() == oracle.get<double>());
        break;
    case true_:
        CHECK(oracle.is_boolean());
        CHECK(oracle.get<bool>() == true);
        break;
    case false_:
        CHECK(oracle.is_boolean());
        CHECK(oracle.get<bool>() == false);
        break;
    case null:
        CHECK(oracle.is_null());
        break;
    default:
        CHECK_MESSAGE(false, "unexpected type %u in json", uint32_t(*val.type()));
        break;
    }
}

enum test_flags : uint32_t {
    precise_real_values = 0b01,
    also_sort_objects   = 0b10,
    skip_dump_compare  = 0b100,

    test_flags_default = 0,
};

template <bool UseCharconv, typename Builder>
pojobuf::value parse_and_get_root(std::string_view json, Builder& builder) {
    auto result = pojobuf::json::parser<UseCharconv>::parse(json, builder);
    REQUIRE(result);
    CHECK(*result == json.data() + json.size());
    auto pl = builder.finalize();
    return pojobuf::value(pl, builder.adata.get_value_buffer_ptr(), builder.abyte.get_byte_ptr());
}

void t(std::string_view json, uint32_t flags = test_flags_default, std::optional<std::string_view> expected_dump = {}) {
    const auto oracle = nljson::parse(json);

    pojobuf::bits::pod_vector buf(pojobuf::json::get_buffer_size_for_json(json));
    pojobuf::bits::pod_vector scratch_buf(pojobuf::json::get_scratch_buffer_size_for_json(json));

    {
        auto doc = pojobuf::document_parse_with<
            pojobuf::json::parser_charconv_num,
            pojobuf::parse_alloc_strategy::take_source
        >(std::string(json));
        REQUIRE(doc);
        rcmp(doc->root(), oracle);
    }
    if ((flags & precise_real_values) == 0) {
        auto json_copy = std::string(json);
        auto data_alloc = pojobuf::alloc::single_buf_nocheck_data_alloc::from_container(buf);
        pojobuf::alloc::mutable_source_byte_alloc byte_alloc(json_copy.data());
        pojobuf::docbuild::buf_builder builder(data_alloc, byte_alloc);
        auto root = parse_and_get_root<false>(json_copy, builder);
        rcmp(root, oracle);
    }

    // same buf, const str
    {
        auto result = pojobuf::document_parse<pojobuf::json::parser_charconv_num>(json);
        REQUIRE(result);
        rcmp(result->root(), oracle);
    }

    // multi buf, const str
    {
        auto data_alloc = pojobuf::alloc::multi_buf_nocheck_data_alloc::from_containers(buf, scratch_buf);
        pojobuf::alloc::valuebuf_byte_alloc byte_alloc(data_alloc);
        pojobuf::docbuild::buf_builder builder(data_alloc, byte_alloc);
        auto root = parse_and_get_root<true>(json, builder);
        rcmp(root, oracle);
    }

    if (flags & also_sort_objects) {
        auto result = pojobuf::document_parse<pojobuf::json::parser_charconv_num>(json, 0);
        REQUIRE(result);
        rcmp(result->root(), oracle);
    }

    // test write

    if (!(flags & skip_dump_compare)) {
        std::string pb_out;
        pojobuf::container_string_sink sink(pb_out);
        pojobuf::json::writer writer(sink);
        pojobuf::json::parser_charconv_num::parse(json, writer);

        if (expected_dump) {
            CHECK(pb_out == *expected_dump);
        }
        else {
            const auto o_out = oracle.dump();
            CHECK(pb_out == o_out);
        }
    }
}

TEST_CASE("successful parse and traverse") {
    t("1");
    t("0.5");
    t("false");
    t("null");
    t("-3");
    t("-3.141592", precise_real_values);
    t("\"a\"");
    t("\"hello\"");
    t("[]");
    t("[1]");
    t("[1,2]");
    t("[1,2,3]");
    t("[1,2,3,4,5,6]");
    t("[[1,2,3,4,5,6,7,8,9,3,4,5,3,1,4,1,5,9,2]]");
    t("[[[[]]]]");
    t("[[[[6]]]]");
    t("[[[[6],[4],[4,1]]]]");
    t("[\"a\"]");
    t("[0,[0,[0],0],0]");
    t("[-2147483648, 2147483647, -2147483649, 2147483648]");
    t(R"({"ar": [2.5, -5], "val": 5, "b": false, "str": "hello world"})", also_sort_objects);
    t(
        R"([1, "hello", -5, 0.25, 1e2, 2.5e-01, 9, {"key": "value", "another_key": 42}, false])",
        also_sort_objects,
        R"([1,"hello",-5,0.25,100,0.25,9,{"key":"value","another_key":42},false])"
    );
    t(R"([
        "easy",    1, -3, -0.25, 1e-010, 1e60, 1e-120,
        "tricky",  1.65, 0.3, 0.333, 3.141592, 3e-121,
        "xtricky", 27.900001108646396, 0.9689776221127033
    ])", precise_real_values);
    t(R"([
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
    ])", also_sort_objects);
    t(R"([
        "foo\tbar",
        "\"\\/\b\f\n\r\t",
        "\ud950\uDf21\n"
    ])");
    t(
        "[3.141592, 4e4, 5.1e-5, 0.3e+2]",
        precise_real_values,
        "[3.141592,40000,5.1e-05,30]"
    );


    auto str = pojobuf::dev::read_file(JSON_TEST_DATA_FILE_github_events_json);

    // this particular file has windows line endings and ends with \r\n
    // to pass our check that the entire string was parsed we need to pop them
    str.pop_back(); str.pop_back();

    t(str, also_sort_objects);
}

void check_find_object_key(pojobuf::value obj, bool expect_sorted) {
    REQUIRE(obj.type().is_object());
    CHECK(obj.type().is_sorted_object() == expect_sorted);

    struct kv { const char* key; int32_t value; };
    const kv present[] = {
        {"zzz", 1}, {"mmm", 2}, {"aaa", 3}, {"qqq", 4}, {"bbb", 5}, {"z", 6},
    };
    for (auto& p : present) {
        const auto i = obj.find_object_key(p.key);
        REQUIRE(i < obj.get_compound_length());
        CHECK(obj.get_object_key(i) == p.key);
        CHECK(obj.get_object_value(i).get_int32_value() == p.value);
    }

    // "": no key is empty
    // "b": same length as the key "z", but different content
    // "bbz": same length as most keys, but different content
    // "zzzz", "aaaaa": lengths which don't match any key
    for (auto missing : {"", "b", "bbz", "zzzz", "aaaaa"}) {
        CHECK(obj.find_object_key(missing) == obj.get_compound_length());
    }
}

TEST_CASE("find_object_key") {
    const char* json = R"({"zzz": 1, "mmm": 2, "aaa": 3, "qqq": 4, "bbb": 5, "z": 6})";

    // unsorted (default): linear scan
    {
        auto doc = pojobuf::document_parse<pojobuf::json::parser_charconv_num>(json);
        REQUIRE(doc);
        check_find_object_key(doc->root(), false);
    }

    // sorted (max_unsorted_obj_records = 0 forces every non-empty object to be sorted):
    // binary search
    {
        auto doc = pojobuf::document_parse<pojobuf::json::parser_charconv_num>(json, 0);
        REQUIRE(doc);
        check_find_object_key(doc->root(), true);
    }
}
