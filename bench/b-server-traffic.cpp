// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// benchmark json parse and then traverse
// a somewhat realistic benchmark of a server that receives json messages and needs to parse and process them

#include "b-traverse-funcs.hpp"

#include <pojobuf/document.hpp>
#include <pojobuf/document_parse.hpp>
#include <pojobuf/json/parser.hpp>

#include <boost/container/pmr/monotonic_buffer_resource.hpp>

#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

#include <pojobuf/dev/read_file.hpp>
#include <json-test-data.h>
#include <string>
#include <vector>

struct input {
    const char* const path;
    std::vector<std::string> lines;
};

const input& get_input(picobench::state& state) {
    auto data = state.input_data();
    return *reinterpret_cast<const input*>(data);
}

/////////////////////////////////
// benchmarks

void bench_pojobuf(picobench::state& state) {
    auto& lines = get_input(state).lines;

    hash_t sum = 0;
    for (auto i : state) {
        auto doc = pojobuf::document_parse<pojobuf::json::parser_custom_num>(lines[i]);
        sum += traverse(doc->root());
    }

    state.set_result(sum);
}

void bench_pojobuf_sep_string(picobench::state& state) {
    auto& lines = get_input(state).lines;

    hash_t sum = 0;
    for (auto i : state) {
        auto doc = pojobuf::document_parse_with<
            pojobuf::json::parser_custom_num,
            pojobuf::parse_alloc_strategy::take_source
        >(std::string(lines[i]));
        sum += traverse(doc->root());
    }

    state.set_result(sum);
}

void bench_sajson(picobench::state& state) {
    auto& lines = get_input(state).lines;

    hash_t sum = 0;
    for (auto i : state) {
        auto doc = sajson::parse(
            sajson::single_allocation{},
            sajson::string{lines[i].data(), lines[i].size()}
        );
        sum += traverse(doc.get_root());
    }

    state.set_result(sum);
}

void bench_boost(picobench::state& state) {
    auto& lines = get_input(state).lines;

    hash_t sum = 0;
    for (auto i : state) {
        auto root = boost::json::parse(lines[i]);
        sum += traverse(root);
    }

    state.set_result(sum);
}

void bench_simdjson(picobench::state& state) {
    auto& lines = get_input(state).lines;

    simdjson::dom::parser p;
    hash_t sum = 0;
    for (auto i : state) {
        simdjson::dom::document doc;
        doc.allocate(lines[i].size());
        p.parse_into_document(doc, lines[i]);
        sum += traverse(doc.root());
    }

    state.set_result(sum);
}

void bench_nlohmann(picobench::state& state) {
    auto& lines = get_input(state).lines;

    hash_t sum = 0;
    for (auto i : state) {
        auto root = nlohmann::json::parse(lines[i]);
        sum += traverse(root);
    }
    state.set_result(sum);
}

int main(int argc, char* argv[]) {
    input inputs[] = {
        {JSON_TEST_DATA_FILE_client_traffic_txt, },
        {JSON_TEST_DATA_FILE_client_traffic_rand_txt, },
    };

    std::vector<picobench::state::input> pb_inputs;
    pb_inputs.reserve(std::size(inputs));

    for (auto& i : inputs) {
        i.lines = pojobuf::dev::read_lines(i.path);
        for (auto& l : i.lines) {
            simdjson::pad(l);
        }
        pb_inputs.push_back({int(i.lines.size()), reinterpret_cast<uintptr_t>(&i)});
    }

    picobench::local_runner r;

    r.add_benchmark("pojobuf", bench_pojobuf).inputs(pb_inputs);
    r.add_benchmark("pojobuf+str", bench_pojobuf_sep_string).inputs(pb_inputs);
    r.add_benchmark("sajson", bench_sajson).inputs(pb_inputs);
    r.add_benchmark("simdjson", bench_simdjson).inputs(pb_inputs);
    r.add_benchmark("boost", bench_boost).inputs(pb_inputs);
    r.add_benchmark("nlohmann", bench_nlohmann).inputs(pb_inputs);

    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    r.parse_cmd_line(argc, argv);

    return r.run();
}