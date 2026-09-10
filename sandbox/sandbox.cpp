// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <pojobuf/document.hpp>
#include <pojobuf/document.parse.hpp>
#include <pojobuf/json/parser.hpp>

#define SAJSON_UNSORTED_OBJECT_KEYS
#include <sajson.h>

#include <pojobuf/dev/read_file.hpp>
#include <json-test-data.h>

#include <chrono>
#include <iostream>
#include <string>
#include <string_view>

#if defined(_MSC_VER)
#define PB_NOINLINE __declspec(noinline)
#else
#define PB_NOINLINE __attribute__((noinline))
#endif

enum class engine {
    pojobuf,
    sajson,
    both,
};

PB_NOINLINE size_t run_pojobuf_once(std::string& content, std::span<std::byte> buf) {
    auto buffer = std::span(reinterpret_cast<std::int64_t*>(buf.data()), buf.size_bytes() / sizeof(std::int64_t));
    auto data_alloc = pojobuf::docbuild::single_buf_nocheck_data_alloc::from_container(buffer);
    pojobuf::docbuild::mutable_source_byte_alloc byte_alloc(content.data());
    pojobuf::docbuild::buf_builder builder(data_alloc, byte_alloc);

    pojobuf::json::parser_custom_num::parse(content, builder);
    auto root = pojobuf::value(builder.finalize(), buffer.data(), content.data());
    return root.get_compound_length();
}

PB_NOINLINE size_t run_sajson_once(std::string& content, std::span<std::byte> buf) {
    auto buffer = std::span(reinterpret_cast<std::size_t*>(buf.data()), buf.size_bytes() / sizeof(std::size_t));
    auto doc = sajson::parse(
        sajson::bounded_allocation{buffer.data(), buffer.size()},
        sajson::mutable_string_view(content.size(), content.data())
    );
    auto root = doc.get_root();
    return root.get_length();
}

const char* resolve_file(std::string_view file) {
    if (file == "canada") return JSON_TEST_DATA_FILE_canada_json;
    if (file == "citm") return JSON_TEST_DATA_FILE_citm_catalog_json;
    if (file == "gsoc") return JSON_TEST_DATA_FILE_gsoc_2018_json;
    if (file == "marine") return JSON_TEST_DATA_FILE_marine_ik_json;
    if (file == "mesh") return JSON_TEST_DATA_FILE_mesh_json;
    if (file == "mesh.pretty") return JSON_TEST_DATA_FILE_mesh_pretty_json;
    return nullptr;
}

engine parse_engine(std::string_view arg) {
    if (arg == "pojobuf") return engine::pojobuf;
    if (arg == "sajson") return engine::sajson;
    if (arg == "both") return engine::both;
    return engine::both;
}

int main(int argc, char** argv) {
    engine eng = engine::both;
    std::string file = "citm";
    int iterations = 200;
    int warmup = 10;

    for (int i = 1; i < argc; ++i) {
        std::string_view a(argv[i]);
        if (a.starts_with("--engine=")) {
            eng = parse_engine(a.substr(9));
        }
        else if (a.starts_with("--file=")) {
            file = a.substr(7);
        }
        else if (a.starts_with("--iters=")) {
            iterations = std::stoi(std::string(a.substr(8)));
        }
        else if (a.starts_with("--warmup=")) {
            warmup = std::stoi(std::string(a.substr(9)));
        }
    }

    std::string path;
    if (auto mapped = resolve_file(file)) {
        path = mapped;
    }
    else {
        path = file;
    }

    const auto input = pojobuf::dev::read_file(path);
    itlib::pod_vector<std::byte> buf(input.size() * sizeof(uint64_t) * 2);
    volatile size_t sink = 0;

    auto run = [&](const char* name, auto fn) {
        for (int i = 0; i < warmup; ++i) {
            auto input_copy = input;
            sink ^= fn(input_copy, buf);
        }
        std::vector<std::string> copies;
        copies.reserve(iterations);
        for (int i= 0; i < iterations; ++i) {
            copies.push_back(input);
        }
        const auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; ++i) {
            sink ^= fn(copies[i], buf);
        }
        const auto t1 = std::chrono::steady_clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        std::cout << name << "\n";
        std::cout << "  file: " << file << "\n";
        std::cout << "  iterations: " << iterations << "\n";
        std::cout << "  total-ms: " << ms << "\n";
        std::cout << "  ns/op: " << (ms * 1000000.0 / iterations) << "\n";
    };

    if (eng == engine::pojobuf || eng == engine::both) {
        run("pojobuf", run_pojobuf_once);
    }
    if (eng == engine::sajson || eng == engine::both) {
        run("sajson", run_sajson_once);
    }

    std::cout << "sink=" << sink << "\n";
    return 0;
}
