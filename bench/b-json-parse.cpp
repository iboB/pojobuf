// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <pojobuf/document.hpp>
#include <pojobuf/document.parse.hpp>
#include <pojobuf/json/parser.hpp>

#define SAJSON_UNSORTED_OBJECT_KEYS
#include <sajson.h>

#include <simdjson.h>

#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

#include <json-test-data.h>
#include <fstream>
#include <string>

struct input {
    const char* const path;
    const char* fname;
    std::string content;
};

const input& get_input(picobench::state& state) {
    auto data = state.input_data();
    return *reinterpret_cast<const input*>(data);
}

template <bool UseCharconv>
void bench_pojobuf_0(picobench::state& state) {
    auto content = get_input(state).content;

    pojobuf::bits::pod_vector buffer(pojobuf::json::get_buffer_size_for_json(content));
    auto data_alloc = pojobuf::docbuild::single_buf_nocheck_data_alloc::from_container(buffer);
    pojobuf::docbuild::mutable_source_byte_alloc byte_alloc(content.data());
    pojobuf::docbuild::buf_builder builder(data_alloc, byte_alloc);

    state.start_timer();
    pojobuf::json::parser<UseCharconv>::parse(content, builder);
    auto root = pojobuf::value(builder.finalize(), buffer.data(), content.data());
    state.stop_timer();

    state.set_result(root.get_compound_length());
}

void bench_sajson_0(picobench::state& state) {
    auto& input = get_input(state);
    auto content = input.content;
    itlib::pod_vector<size_t> buffer(content.size());

    state.start_timer();
    auto doc = sajson::parse(
        sajson::bounded_allocation{buffer.data(), buffer.size()},
        sajson::mutable_string_view(content.size(), content.data())
    );
    auto root = doc.get_root();
    state.stop_timer();

    state.set_result(root.get_length());
}

void bench_simdjson_0(picobench::state& state) {
    auto& content = get_input(state).content;

    simdjson::dom::document doc;
    doc.allocate(content.length());

    state.start_timer();
    simdjson::dom::parser parser;
    auto root = parser.parse_into_document(doc, content);
    state.stop_timer();

    if (root.is_object()) {
        state.set_result(root.get_object().size());
    }
    else if (root.is_array()) {
        state.set_result(root.get_array().size());
    }
}

std::string read_file(const char* path) {
    std::ifstream fin(path);
    if (!fin) {
        throw std::runtime_error("Failed to open file: " + std::string(path));
    }
    std::string content((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    return content;
}

int main(int argc, char* argv[]) {
    //const char* files[] = { JSON_TEST_DATA_JSON_FILES };
    // let's only benchmark the longer files
    // the shorter ones introduce too much noise
    input inputs[] = {
        {JSON_TEST_DATA_FILE_canada_json, },
        {JSON_TEST_DATA_FILE_citm_catalog_json, },
        {JSON_TEST_DATA_FILE_gsoc_2018_json, },
        {JSON_TEST_DATA_FILE_marine_ik_json, },
        {JSON_TEST_DATA_FILE_mesh_json, },
        {JSON_TEST_DATA_FILE_mesh_pretty_json, },
    };

    for (auto& input : inputs) {
        auto sv = std::string_view(input.path);
        auto f = sv.rfind('/');
        if (f != std::string_view::npos) {
            sv = sv.substr(f + 1);
        }
        input.fname = sv.data();

        input.content = read_file(input.path);
        simdjson::pad(input.content);
    }

    picobench::local_runner r;

    for (auto& i : inputs) {
        r.set_suite(i.fname);
        auto add_benchmark = [&](const char* title, auto func) {
            r.add_benchmark(title, func).inputs({{1, reinterpret_cast<uintptr_t>(&i)}});
        };

        add_benchmark("pojobuf-0", bench_pojobuf_0<false>);
        add_benchmark("pojobuf-0 charconv", bench_pojobuf_0<true>);
        add_benchmark("sajson-0", bench_sajson_0);
        add_benchmark("simdjson-0", bench_simdjson_0);
    }

    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    r.parse_cmd_line(argc, argv);

    return r.run();
}
