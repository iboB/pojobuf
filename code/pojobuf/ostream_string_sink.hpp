// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <iosfwd>

namespace pojobuf {

template <typename Stream = std::ostream>
class ostream_string_sink {
public:
    using rdbuf_ptr = decltype(std::declval<Stream>().rdbuf());
    rdbuf_ptr m_rdbuf;

    explicit ostream_string_sink(Stream& s)
        : m_rdbuf(s.rdbuf())
    {}

    void add(char c) {
        m_rdbuf->sputc(c);
    }
    void add(const char* begin, const char* end) {
        add(begin, end - begin);
    }
    void add(const char* begin, size_t size) {
        m_rdbuf->sputn(begin, size);
    }
    void add(std::string_view sv) {
        add(sv.data(), sv.size());
    }
};

} // namespace pojobuf
