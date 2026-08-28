// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "limits.hpp"
#include "../pl_tag.hpp"
#include "../bits/charconv.hpp"
#include <itlib/small_vector.hpp>
#include <concepts>
#include <type_traits>
#include <string_view>
#include <cmath>

namespace pojobuf::json {

namespace util {
// empty return, means no escape
std::string_view escape_utf8_byte(char c) {
    auto u = uint8_t(c);

    // http://www.json.org/
    if (u > '\\') return {}; // no escape needed for characters above backslash
    if (u == '"') return "\\\"";
    if (u == '\\') return "\\\\";
    if (u >= ' ') return {}; // no escape needed for other characters above space
    static constexpr std::string_view below_space[' '] = {
        "\\u0000","\\u0001","\\u0002","\\u0003","\\u0004","\\u0005","\\u0006","\\u0007",
          "\\b"  ,  "\\t"  ,  "\\n"  ,"\\u000b",  "\\f"  ,  "\\r"  ,"\\u000e","\\u000f",
        "\\u0010","\\u0011","\\u0012","\\u0013","\\u0014","\\u0015","\\u0016","\\u0017",
        "\\u0018","\\u0019","\\u001a","\\u001b","\\u001c","\\u001d","\\u001e","\\u001f"
    };
    return below_space[u];
}
} // namespace util

template <typename StringSink, bool GuardNumberValues = false>
class writer {
    bool m_has_value = false;
    bool m_has_added_key = false; // only through builder compat functions
    itlib::small_vector<uint8_t, 32> m_compound_stack;
    uint32_t m_compact_depth;
    std::string_view m_pending_key = {};

    void add_new_line(bool close) const {
        if (!cur_depth_is_pretty()) return;
        const auto indent = cur_depth() - close;
        if (indent == 0 && !m_has_value) return; // no new line for initial value

        // TODO: configurable new line and indent string
        sink.add('\n');
        static constexpr std::string_view tab = "  ";
        for (uint32_t i = 0; i < indent; ++i) {
            sink.add(tab);
        }
    }
public:
    StringSink& sink;

    explicit writer(StringSink& sink, bool pretty = false)
        : m_compact_depth(pretty ? uint32_t(-1) : 0)
        , sink(sink)
    {}

    uint32_t cur_depth() const noexcept {
        return uint32_t(m_compound_stack.size());
    }

    void push_stack(uint8_t t) {
        m_compound_stack.push_back(t);
    }
    void pop_sack() noexcept {
        if (m_compact_depth == cur_depth()) {
            m_compact_depth = uint32_t(-1);
        }
        m_compound_stack.pop_back();
    }

    bool current_compound_is_root() {
        return m_compound_stack.empty();
    }

    bool current_compound_is_array() {
        return m_compound_stack.back() == *pl_tag::array;
    }

    bool current_compound_is_object() {
        auto back = m_compound_stack.back();
        return back == *pl_tag::object || back == *pl_tag::sorted_object;
    }

    void set_render_compact() {
        if (cur_depth_is_pretty()) {
            m_compact_depth = cur_depth();
        }
    }
    bool cur_depth_is_pretty() const noexcept {
        return cur_depth() < m_compact_depth;
    }

    void write_escaped_utf8_string(std::string_view str) {
        // we could use this simple code here
        // but it writes bytes one by one
        //
        // for (auto c : str) {
        //     auto e = escape_utf8_byte(c);
        //     if (!e) sink.add(c);
        //     else sink.add(e);
        // }
        //
        // to optimize, we'll use the following which writes in chunks
        // if there is nothing to be escaped in a string,
        //  it will print the whole string at the end as a single operation

        auto ptr = str.data();
        const auto end = str.data() + str.size();

        auto p = ptr;
        while (p != end) {
            auto esc = util::escape_utf8_byte(*p);
            if (!esc.data()) { // somewhat hacky rely on empty returns to have nullptr data
                ++p;
            }
            else {
                if (p != ptr) sink.add(ptr, p);
                sink.add(esc);
                ptr = ++p;
            }
        }

        if (p != ptr) {
            sink.add(ptr, p);
        }
    }

    void write_quoted_escaped_ut8_string(std::string_view str) {
        sink.add('"');
        write_escaped_utf8_string(str);
        sink.add('"');
    }

    void add_raw_json_value(std::string_view& str) {
        prepare_for_val();
        sink.add(str);
    }

    void add_unescaped_string_value(std::string_view& str) {
        prepare_for_val();
        sink.add('"');
        sink.add(str);
        sink.add('"');
    }

    template <pl_tag Tag>
    void add_literal_element() {
        prepare_for_val();

        if constexpr (Tag == pl_tag::null) {
            sink.add("null", 4);
        }
        else if constexpr (Tag == pl_tag::false_) {
            sink.add("false", 5);
        }
        else if constexpr (Tag == pl_tag::true_) {
            sink.add("true", 4);
        }
        else {
            static_assert(Tag == pl_tag::null, "unsupported literal tag");
        }
    }

    template <std::integral I>
    bool add_number_element(I i) {
        if constexpr (GuardNumberValues && sizeof(I) > 4) { // all ints of 32 bits and below fit a double
            if constexpr (std::is_signed_v<I>) {
                if (i < min_int64 || i > max_int64) {
                    return false;
                }
            }
            else if (i > max_uint64) {
                return false;
            }
        }

        prepare_for_val();

        // instead of using charconv or similar we can make use of some facts to make this more optimial
        // * base 10 is known at compile time
        // * we don't need to fill the front of a buffer, instead we can start from the back and output reverse

        using unsigned_t = std::make_unsigned_t<I>;
        unsigned_t u = unsigned_t(i);

        if constexpr (std::is_signed_v<I>) {
            if (i < 0) {
                sink.add('-');
                u = 0 - u;
            }
        }

        char buf[24]; // enough for signed 2^64 in decimal
        const auto end = buf + sizeof(buf);
        auto p = end;

        do {
            *--p = char('0' + u % 10);
            u /= 10;
        } while (u != 0);

        sink.add(p, end);

        return true;
    }

    template <std::floating_point F>
    bool add_number_element(F f) {
        if constexpr (GuardNumberValues) {
            if (!std::isfinite(f)) {
                return false;
            }
        }

        prepare_for_val();

        char out[25]; // max length of double
        auto result = POJOBUF_CHARCONV_NAMESPACE::to_chars(out, out + sizeof(out), f);
        sink.add(out, result.ptr);

        return true;
    }

    void add_string_element(std::string_view str) {
        prepare_for_val();
        write_quoted_escaped_ut8_string(str);
    }

    template <pl_tag Tag>
    void open_compound_element() {
        prepare_for_val();

        if constexpr (Tag == pl_tag::array) {
            sink.add('[');
        }
        else if constexpr (Tag == pl_tag::object) {
            sink.add('{');
        }
        else {
            static_assert(Tag == pl_tag::array, "unsupported compound element");
        }

        m_has_value = false;
        push_stack(uint8_t(Tag));
    }

    void add_object_key(std::string_view str) {
        m_pending_key = str;
    }

    void discard_pending_key() noexcept {
        m_pending_key = {};
    }

    template <pl_tag Tag>
    void close_compound_element() {
        if (m_has_value) {
            add_new_line(true);
        }

        if constexpr (Tag == pl_tag::array) {
            sink.add(']');
        }
        else if constexpr (Tag == pl_tag::object) {
            sink.add('}');
        }
        else {
            static_assert(Tag == pl_tag::array, "unsupported compound element");
        }

        m_has_value = true;
        pop_sack();
    }

    void prepare_for_val() {
        if (m_has_added_key) {
            m_has_added_key = false;
            return;
        }

        if (m_has_value) {
            sink.add(',');
        }

        add_new_line(false);

        if (m_pending_key.data()) {
            write_quoted_escaped_ut8_string(m_pending_key);
            sink.add(':');
            m_pending_key = {};
        }

        m_has_value = true;
    }

    // parser compat
    buffer_range push_string(const char* begin, const char* end) {
        prepare_for_val();
        write_quoted_escaped_ut8_string({begin, end});
        return {};
    }
    buffer_range push_internal_string(const char* begin, const char* end) {
        prepare_for_val();
        sink.add('"');
        sink.add(begin, end);
        sink.add('"');
        return {};
    }

    struct piecewise_string_builder {
        StringSink& sink;
        void push(char c) {
            auto e = util::escape_utf8_byte(c);
            if (e.data()) {
                sink.add(e);
            }
            else {
                sink.add(c);
            }
        }
    };
    piecewise_string_builder get_piecewise_string_builder(const char* simple_begin, const char* begin) {
        prepare_for_val();
        sink.add('"');
        sink.add(simple_begin, begin);
        return piecewise_string_builder{sink};
    }
    buffer_range push_string(const piecewise_string_builder& psb) noexcept {
        sink.add('"');
        return {};
    }

    void add_string_element(const buffer_range&) {
        // nothing to do here since the job has been done by push_string
    }
    void add_object_key(const buffer_range&) {
        // the string itself has been added by push_string
        sink.add(':');
        m_has_added_key = true;
    }
};

} // namespcae pojobuf::json
