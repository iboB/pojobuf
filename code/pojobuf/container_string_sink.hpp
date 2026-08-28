// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <string_view>

namespace pojobuf {

template <typename Container>
class container_string_sink {
public:
    Container& container;

    explicit container_string_sink(Container& c)
        : container(c)
    {}

    void add(char c) {
        container.push_back(c);
    }
    void add(const char* begin, const char* end) {
        container.insert(container.end(), begin, end);
    }
    void add(const char* begin, size_t size) {
        add(begin, begin + size);
    }
    void add(std::string_view sv) {
        add(sv.data(), sv.size());
    }
};

} // namespace pojobuf
