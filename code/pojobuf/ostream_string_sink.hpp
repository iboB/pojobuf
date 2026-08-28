// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <iosfwd>

namespace pojobuf {

template <typename Stream = std::ostream>
class ostream_string_sink {
public:
    Stream& stream;

    explicit ostream_string_sink(Stream& s)
        : stream(s)
    {}

    void add(char c) {
        stream.rdbuf()->sputc(c);
    }
    void add(const char* begin, const char* end) {
        add(begin, end - begin);
    }
    void add(const char* begin, size_t size) {
        stream.rdbuf()->sputn(begin, size);
    }
    void add(std::string_view sv) {
        add(sv.data(), sv.size());
    }
};

} // namespace pojobuf
