#include <pojobuf/document.hpp>
#include <pojobuf/document.parse.hpp>
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
    std::vector<std::string> lines;
};

const input& get_input(picobench::state& state) {
    auto data = state.input_data();
    return *reinterpret_cast<const input*>(data);
}

int main() {
    input inputs[] = {
        {JSON_TEST_DATA_FILE_client_traffic_txt, }
    };

    std::vector<picobench::state::input> pb_inputs;
    pb_inputs.reserve(std::size(inputs));

    for (auto& i : inputs) {
        i.lines = pojobuf::dev::read_lines(i.path);
        pb_inputs.push_back({int(i.lines.size()), reinterpret_cast<uintptr_t>(&i)});
    }



}