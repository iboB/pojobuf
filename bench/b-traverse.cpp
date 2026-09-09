#include <pojobuf/document.hpp>
#include <pojobuf/document.parse.hpp>
#include <pojobuf/json/parser.hpp>

#include <boost/json.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>

#define SAJSON_UNSORTED_OBJECT_KEYS
#include <sajson.h>

#include <simdjson.h>

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

using hash_t = uint64_t;

struct vec {
    int x, y, z;
    hash_t sum() const { return hash_t(x + y + z); }
};

hash_t hash(std::string_view str) {
    return std::hash<std::string_view>{}(str);
}

///////////////////////
// pojobuf

hash_t check_ack(const pojobuf::value& obj) {
    auto f = obj.find_object_key("ack");
    if (f == obj.get_compound_length()) return 0;
    return obj.get_object_value(f).get_int32_value();
}

hash_t check_setSubscriptions(const pojobuf::value& obj) {
    auto f = obj.find_object_key("setSubscriptions");
    if (f == obj.get_compound_length()) return 0;
    hash_t res = 0;
    auto subs = obj.get_object_value(f);
    const size_t len = subs.get_compound_length();
    for (size_t i = 0; i < len; ++i) {
        auto key = subs.get_object_key(i);
        auto value = subs.get_object_value(i).get_string_value();
        res += hash(key);
        res += hash(value);
    }
    return res;
}

vec read_vec(const pojobuf::value& obj) {
    return {
        obj.get_object_value(obj.find_object_key("x")).get_int32_value(),
        obj.get_object_value(obj.find_object_key("y")).get_int32_value(),
        obj.get_object_value(obj.find_object_key("z")).get_int32_value(),
    };
}

hash_t check_setRequestBatch(const pojobuf::value& obj) {
    auto f = obj.find_object_key("setRequestBatch");
    if (f == obj.get_compound_length()) return 0;
    hash_t res = 0;
    auto batch = obj.get_object_value(f);

    {
        auto batchId = batch.get_object_value(batch.find_object_key("batchID")).get_string_value();
        res += hash(batchId);
    }

    auto reqs = batch.get_object_value(batch.find_object_key("requests"));
    auto rlen = reqs.get_compound_length();

    for (size_t i = 0; i < rlen; ++i) {
        auto req = reqs.get_object_value(i);
        res += read_vec(req.get_object_value(req.find_object_key("min"))).sum();
        res += read_vec(req.get_object_value(req.find_object_key("max"))).sum();
    }

    return res;
}

hash_t check_ping(const pojobuf::value& obj) {
    auto f = obj.find_object_key("ping");
    if (f == obj.get_compound_length()) return 0;
    auto ping = obj.get_object_value(f);

    auto pl = ping.get_object_value(ping.find_object_key("payload")).get_string_value();
    return hash(pl);
}

hash_t check_setInteraction(const pojobuf::value& obj) {
    auto f = obj.find_object_key("setInteraction");
    if (f == obj.get_compound_length()) return 0;
    hash_t res = 0;
    auto i = obj.get_object_value(f);

    auto type = i.get_object_value(i.find_object_key("type")).get_string_value();
    if (type != "PlanarDrag_World") return 42;

    res += i.get_object_value(i.find_object_key("done")).get_boolean_value();
    res += i.get_object_value(i.find_object_key("confirm")).get_boolean_value();

    auto tool = i.get_object_value(i.find_object_key("tool")).get_string_value();
    res += hash(tool);

    res += i.get_object_value(i.find_object_key("id")).get_int32_value();
    res += i.get_object_value(i.find_object_key("seq")).get_int32_value();

    auto ss = i.get_object_value(i.find_object_key("structures"));
    auto sslen = ss.get_compound_length();
    for (size_t j = 0; j < sslen; ++j) {
        auto s = ss.get_array_element(j).get_string_value();
        res += hash(s);
    }

    return res;
}

hash_t traverse(const pojobuf::value& val) {
    hash_t res = 0;

    res += check_ack(val);
    res += check_setSubscriptions(val);
    res += check_setRequestBatch(val);
    res += check_ping(val);
    res += check_setInteraction(val);
    return res;
}

/////////////////////////////////
// sajson

std::string_view to_sv(const sajson::value& val) {
    return std::string_view(val.as_cstring(), val.get_string_length());
}

std::string_view to_sv(const sajson::string& str) {
    return std::string_view(str.data(), str.length());
}

hash_t check_ack(const sajson::value& obj) {
    auto f = obj.find_object_key(sajson::literal("ack"));
    if (f == obj.get_length()) return 0;
    return obj.get_object_value(f).get_integer_value();
}

hash_t check_setSubscriptions(const sajson::value& obj) {
    auto f = obj.find_object_key(sajson::literal("setSubscriptions"));
    if (f == obj.get_length()) return 0;
    hash_t res = 0;
    auto subs = obj.get_object_value(f);
    const size_t len = subs.get_length();
    for (size_t i = 0; i < len; ++i) {
        auto key = to_sv(subs.get_object_key(i));
        auto value = to_sv(subs.get_object_value(i));
        res += hash(key);
        res += hash(value);
    }
    return res;
}

vec read_vec(const sajson::value& obj) {
    return {
        obj.get_object_value(obj.find_object_key(sajson::literal("x"))).get_integer_value(),
        obj.get_object_value(obj.find_object_key(sajson::literal("y"))).get_integer_value(),
        obj.get_object_value(obj.find_object_key(sajson::literal("z"))).get_integer_value(),
    };
}

hash_t check_setRequestBatch(const sajson::value& obj) {
    auto f = obj.find_object_key(sajson::literal("setRequestBatch"));
    if (f == obj.get_length()) return 0;
    hash_t res = 0;
    auto batch = obj.get_object_value(f);

    {
        auto batchId = to_sv(batch.get_object_value(batch.find_object_key(sajson::literal("batchID"))));
        res += hash(batchId);
    }

    auto reqs = batch.get_object_value(batch.find_object_key(sajson::literal("requests")));
    auto rlen = reqs.get_length();

    for (size_t i = 0; i < rlen; ++i) {
        auto req = reqs.get_object_value(i);
        res += read_vec(req.get_object_value(req.find_object_key(sajson::literal("min")))).sum();
        res += read_vec(req.get_object_value(req.find_object_key(sajson::literal("max")))).sum();
    }

    return res;
}

hash_t check_ping(const sajson::value& obj) {
    auto f = obj.find_object_key(sajson::literal("ping"));
    if (f == obj.get_length()) return 0;
    auto ping = obj.get_object_value(f);

    auto pl = to_sv(ping.get_object_value(ping.find_object_key(sajson::literal("payload"))));
    return hash(pl);
}

hash_t check_setInteraction(const sajson::value& obj) {
    auto f = obj.find_object_key(sajson::literal("setInteraction"));
    if (f == obj.get_length()) return 0;
    hash_t res = 0;
    auto i = obj.get_object_value(f);

    auto type = to_sv(i.get_object_value(i.find_object_key(sajson::literal("type"))));
    if (type != "PlanarDrag_World") return 42;

    res += i.get_object_value(i.find_object_key(sajson::literal("done"))).get_boolean_value();
    res += i.get_object_value(i.find_object_key(sajson::literal("confirm"))).get_boolean_value();

    auto tool = to_sv(i.get_object_value(i.find_object_key(sajson::literal("tool"))));
    res += hash(tool);

    res += i.get_object_value(i.find_object_key(sajson::literal("id"))).get_integer_value();
    res += i.get_object_value(i.find_object_key(sajson::literal("seq"))).get_integer_value();

    auto ss = i.get_object_value(i.find_object_key(sajson::literal("structures")));
    auto sslen = ss.get_length();
    for (size_t j = 0; j < sslen; ++j) {
        auto s = to_sv(ss.get_array_element(j));
        res += hash(s);
    }

    return res;
}

hash_t traverse(const sajson::value& val) {
    hash_t res = 0;

    res += check_ack(val);
    res += check_setSubscriptions(val);
    res += check_setRequestBatch(val);
    res += check_ping(val);
    res += check_setInteraction(val);
    return res;
}

/////////////////////////////////
// simdjson

vec read_vec(simdjson::dom::object& obj) {
    vec v;
    v.x = int(obj.at_key("x").get_int64().value_unsafe());
    v.y = int(obj.at_key("y").get_int64().value_unsafe());
    v.z = int(obj.at_key("z").get_int64().value_unsafe());
    return v;
}

hash_t check_ack(simdjson::dom::object& obj) {
    auto ack = obj.at_key("ack");
    if (ack.error()) return 0;
    return ack.get_int64().value_unsafe();
}

hash_t check_setSubscriptions(simdjson::dom::object& obj) {
    auto subs = obj.at_key("setSubscriptions");
    if (subs.error()) return 0;
    hash_t res = 0;
    auto subobj = subs.get_object().value_unsafe();
    for (auto& field : subobj) {
        res += hash(field.key);
        res += hash(field.value.get_string().value_unsafe());
    }
    return res;
}

hash_t check_setRequestBatch(simdjson::dom::object& obj) {
    auto batch = obj.at_key("setRequestBatch");
    if (batch.error()) return 0;
    hash_t res = 0;
    auto batchobj = batch.get_object().value_unsafe();
    {
        std::string_view batchId = batchobj.at_key("batchID").get_string().value_unsafe();
        res += hash(batchId);
    }
    auto reqs = batchobj.at_key("requests").get_object().value_unsafe();
    for (auto& reqfield : reqs) {
        auto req = reqfield.value.get_object().value_unsafe();
        {
            auto minobj = req.at_key("min").get_object().value_unsafe();
            res += read_vec(minobj).sum();
        }
        {
            auto maxobj = req.at_key("max").get_object().value_unsafe();
            res += read_vec(maxobj).sum();
        }
    }
    return res;
}

hash_t check_ping(simdjson::dom::object& obj) {
    auto ping = obj.at_key("ping");
    if (ping.error()) return 0;
    auto pingobj = ping.get_object().value_unsafe();
    std::string_view pl = pingobj.at_key("payload").get_string().value_unsafe();
    return hash(pl);
}

hash_t check_setInteraction(simdjson::dom::object& obj) {
    auto inter = obj.at_key("setInteraction");
    if (inter.error()) return 0;
    hash_t res = 0;
    auto interobj = inter.get_object().value_unsafe();
    std::string_view type = interobj.at_key("type").get_string().value_unsafe();
    if (type != "PlanarDrag_World") return 42;
    bool done = interobj.at_key("done").get_bool().value_unsafe();
    bool confirm = interobj.at_key("confirm").get_bool().value_unsafe();
    res += done + confirm;
    std::string_view tool = interobj.at_key("tool").get_string().value_unsafe();
    res += hash(tool);
    int id = int(interobj.at_key("id").get_int64().value_unsafe());
    int seq = int(interobj.at_key("seq").get_int64().value_unsafe());
    res += id + seq;
    auto structures = interobj.at_key("structures").get_array().value_unsafe();
    for (auto sv : structures) {
        std::string_view sid = sv.get_string().value_unsafe();
        res += hash(sid);
    }
    return res;
}

hash_t traverse(const simdjson::dom::element& elem) {
    auto obj = elem.get_object().value_unsafe();
    hash_t res = 0;
    res += check_ack(obj);
    res += check_setSubscriptions(obj);
    res += check_setRequestBatch(obj);
    res += check_ping(obj);
    res += check_setInteraction(obj);
    return res;
}

/////////////////////////////////
// boost

vec read_vec(boost::json::object& obj) {
    vec v;
    v.x = int(obj["x"].as_int64());
    v.y = int(obj["y"].as_int64());
    v.z = int(obj["z"].as_int64());
    return v;
}

hash_t check_ack(boost::json::object& obj) {
    auto it = obj.find("ack");
    if (it == obj.end()) return 0;
    return it->value().as_int64();
}

hash_t check_setSubscriptions(boost::json::object& obj) {
    auto it = obj.find("setSubscriptions");
    if (it == obj.end()) return 0;
    hash_t res = 0;
    auto subobj = it->value().as_object();
    for (auto& kv : subobj) {
        res += hash(kv.key());
        res += hash(kv.value().as_string());
    }
    return res;
}

hash_t check_setRequestBatch(boost::json::object& obj) {
    auto it = obj.find("setRequestBatch");
    if (it == obj.end()) return 0;
    hash_t res = 0;
    auto batchobj = it->value().as_object();
    {
        std::string_view batchId = batchobj["batchID"].as_string();
        res += hash(batchId);
    }
    auto reqs = batchobj["requests"].as_object();
    for (auto& reqv : reqs) {
        auto req = reqv.value().as_object();
        {
            auto minobj = req["min"].as_object();
            res += read_vec(minobj).sum();
        }
        {
            auto maxobj = req["max"].as_object();
            res += read_vec(maxobj).sum();
        }
    }
    return res;
}

hash_t check_ping(boost::json::object& obj) {
    auto it = obj.find("ping");
    if (it == obj.end()) return 0;
    auto pingobj = it->value().as_object();
    std::string_view pl = pingobj["payload"].as_string();
    return hash(pl);
}

hash_t check_setInteraction(boost::json::object& obj) {
    auto it = obj.find("setInteraction");
    if (it == obj.end()) return 0;
    hash_t res = 0;
    auto interobj = it->value().as_object();
    std::string_view type = interobj["type"].as_string();
    if (type != "PlanarDrag_World") return 42;
    bool done = interobj["done"].as_bool();
    bool confirm = interobj["confirm"].as_bool();
    res += done + confirm;
    std::string_view tool = interobj["tool"].as_string();
    res += hash(tool);
    int id = int(interobj["id"].as_int64());
    int seq = int(interobj["seq"].as_int64());
    res += id + seq;
    auto structures = interobj["structures"].as_array();
    for (auto& sv : structures) {
        std::string_view sid = sv.as_string();
        res += hash(sid);
    }
    return res;
}

hash_t traverse(boost::json::value& val) {
    auto obj = val.as_object();
    hash_t res = 0;
    res += check_ack(obj);
    res += check_setSubscriptions(obj);
    res += check_setRequestBatch(obj);
    res += check_ping(obj);
    res += check_setInteraction(obj);
    return res;
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

void bench_pojobuf_sep_string(picobench::state& state) {
    auto& lines = get_input(state).lines;
    std::vector<pojobuf::document<std::string>> docs;
    docs.reserve(lines.size());
    for (auto& l : lines) {
        docs.emplace_back(
            *pojobuf::document_parse_with<
                pojobuf::json::parser_custom_num,
                pojobuf::parse_alloc_strategy::take_source
            >(std::string(l))
        );
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

    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    r.parse_cmd_line(argc, argv);

    return r.run();
}