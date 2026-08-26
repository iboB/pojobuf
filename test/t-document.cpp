// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <pojobuf/document.hpp>
#include <pojobuf/document.parse.hpp>
#include <pojobuf/json/parser.hpp>

#include <doctest/doctest.h>

const std::string_view json = R"({"ar": [2.3, -5], "val": 5, "b": false, "str": "hello world"})";

template <typename B>
using parser = pojobuf::json::parser_charconv_num<B>;
using as = pojobuf::doc_alloc_strategy;

TEST_CASE("embedded bytes") {
    auto doc = pojobuf::document_parse<parser>(json);
    REQUIRE(doc);
    CHECK(doc->alloc_strategy() == as::embed_bytes_in_data);
    const auto doc_buf_ptr = reinterpret_cast<const void*>(doc->buffer().data());
    CHECK(doc_buf_ptr == doc->byte_ptr());

    CHECK(doc->root().get_object_key(0) == "ar");

    auto copy = doc->copy();
    CHECK(copy.buffer() == doc->buffer());
    CHECK(copy.buffer().data() != doc->buffer().data());
    CHECK(reinterpret_cast<const void*>(copy.byte_ptr()) == copy.buffer().data());

    CHECK(copy.root().get_object_key(0) == "ar");

    auto moved = std::move(*doc);
    CHECK(moved.buffer() == copy.buffer());
    CHECK(doc_buf_ptr == moved.buffer().data());

    CHECK(moved.root().get_object_key(0) == "ar");
}

TEST_CASE("external bytes") {
    auto json_copy = std::string(json);
    auto doc = pojobuf::document_parse<parser>(as::external_mutable_source, json_copy);
    CHECK(json_copy != json); // string was modified by the parser
    CHECK(doc->alloc_strategy() == as::external_mutable_source);

    CHECK(doc->byte_ptr() == json_copy.data());

    const auto doc_buf_ptr = doc->buffer().data();

    CHECK(doc->root().get_object_key(0) == "ar");

    auto copy = doc->copy();
    CHECK(copy.buffer() == doc->buffer());
    CHECK(copy.buffer().data() != doc->buffer().data());
    CHECK(copy.byte_ptr() == json_copy.data());

    CHECK(copy.root().get_object_key(0) == "ar");

    auto moved = std::move(*doc);
    CHECK(moved.buffer() == copy.buffer());
    CHECK(doc_buf_ptr == moved.buffer().data());

    CHECK(moved.root().get_object_key(0) == "ar");

    // hax
    json_copy[2] = 'X';
    json_copy[3] = 'Y';

    CHECK(moved.root().get_object_key(0) == "XY");
    CHECK(copy.root().get_object_key(0) == "XY");
}

TEST_CASE("embedded bytes") {
    auto doc = pojobuf::document_parse<parser>(as::take_source, std::string(json));
    CHECK(doc->alloc_strategy() == as::take_source);

    const auto doc_buf_ptr = doc->buffer().data();
    const auto doc_byte_ptr = doc->byte_ptr();

    CHECK(reinterpret_cast<const char*>(doc_buf_ptr) != doc_byte_ptr);
    CHECK(doc_byte_ptr == doc->byte_buf().data());

    CHECK(doc->root().get_object_key(0) == "ar");

    auto copy = doc->copy();
    CHECK(copy.buffer() == doc->buffer());
    CHECK(copy.buffer().data() != doc->buffer().data());
    CHECK(copy.byte_buf() == doc->byte_buf());
    CHECK(copy.byte_buf().data() != doc->byte_buf().data());

    CHECK(copy.root().get_object_key(0) == "ar");

    auto moved = std::move(*doc);
    CHECK(moved.buffer() == copy.buffer());
    CHECK(moved.byte_buf() == copy.byte_buf());
    CHECK(doc_buf_ptr == moved.buffer().data());
    CHECK(doc_byte_ptr == moved.byte_buf().data());

    CHECK(moved.root().get_object_key(0) == "ar");
}
