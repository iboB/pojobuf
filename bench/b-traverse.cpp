// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// benchmarking traverse only

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
    std::vector<pojobuf::document<>> docs;
    docs.reserve(lines.size());
    for (auto& l : lines) {
        docs.emplace_back(*pojobuf::document_parse<pojobuf::json::parser_custom_num>(l));
    }

    hash_t sum = 0;
    for (auto i : state) {
        sum += traverse(docs[i].root());
    }

    state.set_result(sum);
}

void bench_sajson(picobench::state& state) {
    auto& lines = get_input(state).lines;
    std::vector<sajson::document> docs;
    docs.reserve(lines.size());
    for (auto& l : lines) {
        docs.emplace_back(sajson::parse(
            sajson::single_allocation{},
            sajson::string{l.data(), l.size()}
        ));
    }

    hash_t sum = 0;
    for (auto i : state) {
        sum += traverse(docs[i].get_root());
    }

    state.set_result(sum);
}

void bench_boost(picobench::state& state) {
    auto& lines = get_input(state).lines;
    std::vector<boost::json::value> roots;
    roots.reserve(lines.size());
    for (auto& l : lines) {
        roots.push_back(boost::json::parse(l));
    }

    hash_t sum = 0;
    for (auto i : state) {
        sum += traverse(roots[i]);
    }

    state.set_result(sum);
}

void bench_simdjson(picobench::state& state) {
    auto& lines = get_input(state).lines;
    std::vector<simdjson::dom::document> docs;
    simdjson::dom::parser p;
    docs.reserve(lines.size());
    for (auto& l : lines) {
        auto& doc = docs.emplace_back();
        std::ignore = doc.allocate(l.size());
        p.parse_into_document(doc, l);
    }

    hash_t sum = 0;
    for (auto i : state) {
        sum += traverse(docs[i].root());
    }

    state.set_result(sum);
}

void bench_nlohmann(picobench::state& state) {
    auto& lines = get_input(state).lines;
    std::vector<nlohmann::json> roots;
    roots.reserve(lines.size());
    for (auto& l : lines) {
        roots.push_back(nlohmann::json::parse(l));
    }
    hash_t sum = 0;
    for (auto i : state) {
        sum += traverse(roots[i]);
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
    r.add_benchmark("sajson", bench_sajson).inputs(pb_inputs);
    r.add_benchmark("simdjson", bench_simdjson).inputs(pb_inputs);
    r.add_benchmark("boost", bench_boost).inputs(pb_inputs);
    r.add_benchmark("nlohmann", bench_nlohmann).inputs(pb_inputs);

    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    r.parse_cmd_line(argc, argv);

    return r.run();
}