// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <pojobuf/document.hpp>
#include <pojobuf/document_parse.hpp>
#include <pojobuf/json/parser.hpp>

#define SAJSON_UNSORTED_OBJECT_KEYS
#include <sajson.h>

#include <simdjson.h>

#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

#include <pojobuf/dev/read_file.hpp>
#include <json-test-data.h>
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

// pojobuf and sajson can parse in place: given a mutable source buffer they own, they reuse
// its bytes directly (no internal copy of string content). Given a const/borrowed source, they
// have to copy string bytes into their own storage instead. This is a genuine behavioral
// difference with a real, measurable cost (an extra O(source size) copy), so we benchmark both:
// "mut" hoists ownership of a mutable copy of the source outside the timer (measuring parsing
// only, no copy); "const" passes a const/borrowed source, so the internal copy happens inside
// the timed region.
//
// simdjson's dom API has no such distinction: it always copies string content into its own
// owned buffers regardless of whether the source is mutable, so there's nothing to compare
// there -- see the single "simdjson" benchmark below.
//
// We don't separately benchmark allocation cost (i.e. buffer preallocated vs allocated by the
// library) any more. In a tight, repeated-call harness like picobench, the system allocator
// serves same-sized alloc/free pairs from its free lists with no syscalls and already-resident
// pages, so that cost is not reliably measurable this way.

template <bool UseCharconv>
void bench_pojobuf_mut(picobench::state& state) {
    auto content = get_input(state).content;

    pojobuf::bits::pod_vector buffer(pojobuf::json::get_buffer_size_for_json(content));
    auto data_alloc = pojobuf::alloc::single_buf_nocheck_data_alloc::from_container(buffer);
    pojobuf::alloc::mutable_source_byte_alloc byte_alloc(content.data());
    pojobuf::buf_builder builder(data_alloc, byte_alloc);

    state.start_timer();
    pojobuf::json::parser<UseCharconv>::parse(content, builder);
    auto root = pojobuf::value(builder.finalize(), buffer.data(), content.data());
    state.stop_timer();

    state.set_result(root.get_compound_length());
}

void bench_pojobuf_const(picobench::state& state) {
    auto& content = get_input(state).content;

    state.start_timer();
    auto doc = pojobuf::document_parse<pojobuf::json::parser_custom_num>(content);
    auto root = doc->root();
    state.stop_timer();

    state.set_result(root.get_compound_length());
}

void bench_sajson_mut(picobench::state& state) {
    auto content = get_input(state).content;

    // note that we use the noinit allocator here
    // if we don't, pod_vector will zero-init the memory which incidentally also warms it up
    // thus the result from parse below becomes significantly faster as the buffer is in cache
    itlib::pod_vector<size_t, pojobuf::bits::noinit_pod_allocator> buffer(content.size());

    state.start_timer();
    auto doc = sajson::parse(
        sajson::bounded_allocation{buffer.data(), buffer.size()},
        sajson::mutable_string_view(content.size(), content.data())
    );
    auto root = doc.get_root();
    state.stop_timer();

    state.set_result(root.get_length());
}

void bench_sajson_const(picobench::state& state) {
    auto& content = get_input(state).content;

    state.start_timer();
    auto doc = sajson::parse(
        sajson::single_allocation{},
        sajson::string{content.data(), content.size()}
    );
    auto root = doc.get_root();
    state.stop_timer();

    state.set_result(root.get_length());
}

// simdjson's dom API always copies string content into its own owned document buffer, whether
// the source we hand it is mutable or not, so there's no mut/const split to benchmark here. We
// just hoist all allocation (both the document's tape/string buffer and the parser's own
// internal stage-1 buffers) out of the timed region, to measure parsing alone.
void bench_simdjson(picobench::state& state) {
    auto& content = get_input(state).content;

    simdjson::dom::document doc;
    simdjson::dom::parser parser;
    std::ignore = doc.allocate(content.length());
    std::ignore = parser.allocate(content.length());

    state.start_timer();
    auto root = parser.parse_into_document(doc, content);
    state.stop_timer();

    if (root.is_object()) {
        state.set_result(root.get_object().size());
    }
    else if (root.is_array()) {
        state.set_result(root.get_array().size());
    }
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

        input.content = pojobuf::dev::read_file(input.path);
        simdjson::pad(input.content);
    }

    picobench::local_runner r;

    for (auto& i : inputs) {
        r.set_suite(i.fname);
        auto add_benchmark = [&](const char* title, auto func) {
            r.add_benchmark(title, func).inputs({{1, reinterpret_cast<uintptr_t>(&i)}});
        };

        add_benchmark("pojobuf-mut", bench_pojobuf_mut<false>);
        add_benchmark("pojobuf-mut charconv", bench_pojobuf_mut<true>);
        add_benchmark("sajson-mut", bench_sajson_mut);
        add_benchmark("pojobuf-const", bench_pojobuf_const);
        add_benchmark("sajson-const", bench_sajson_const);
        add_benchmark("simdjson", bench_simdjson);
    }

    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    r.parse_cmd_line(argc, argv);

    return r.run();
}
