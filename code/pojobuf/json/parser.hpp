// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "util.hpp"
#include "../parse_error.hpp"
#include "../parse_error.create.hpp"
#include "../pl_tag.hpp"
#include "../buffer_range.hpp"
#include "../bits/charconv.hpp"

#include <itlib/expected.hpp>

#include <limits>
#include <string_view>

#include <splat/inline.h>

namespace pojobuf::json {

using errc = parse_error::errc;

struct t_parser_base {
    // bit 0 (1) - plain ASCII string character
    // bit 1 (2) - whitespace
    // bit 3 (4) - numeric
    static constexpr const uint8_t parse_flags[256] = {
     // 0    1    2    3    4    5    6    7      8    9    A    B    C    D    E    F
        0,   0,   0,   0,   0,   0,   0,   0,     0,   2,   2,   0,   0,   2,   0,   0, // 0
        0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, // 1
        3,   1,   0,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,   1,   0x11,1, // 2
        0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,  0x11,0x11,1,   1,   1,   1,   1,   1, // 3
        1,   1,   1,   1,   1,   0x11,1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 4
        1,   1,   1,   1,   1,   1,   1,   1,     1,   1,   1,   1,   0,   1,   1,   1, // 5
        1,   1,   1,   1,   1,   0x11,1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 6
        1,   1,   1,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 7

        // 128-255
        // implicit zeros
    };

    static FORCE_INLINE bool is_plain_string_character(char c) noexcept {
        return (parse_flags[uint8_t(c)] & 1) != 0;
    }
    static FORCE_INLINE bool is_whitespace(char c) noexcept {
        return (parse_flags[uint8_t(c)] & 2) != 0;
    }

    static double pow10(int64_t exponent) {
        if (exponent > 308) [[unlikely]] {
            return std::numeric_limits<double>::infinity();
        }
        else if (exponent < -323) [[unlikely]] {
            return 0.0;
        }

        // clang-format off
        static constexpr double constants[] = {
            1e-323,1e-322,1e-321,1e-320,1e-319,1e-318,1e-317,1e-316,1e-315,1e-314,
            1e-313,1e-312,1e-311,1e-310,1e-309,1e-308,1e-307,1e-306,1e-305,1e-304,
            1e-303,1e-302,1e-301,1e-300,1e-299,1e-298,1e-297,1e-296,1e-295,1e-294,
            1e-293,1e-292,1e-291,1e-290,1e-289,1e-288,1e-287,1e-286,1e-285,1e-284,
            1e-283,1e-282,1e-281,1e-280,1e-279,1e-278,1e-277,1e-276,1e-275,1e-274,
            1e-273,1e-272,1e-271,1e-270,1e-269,1e-268,1e-267,1e-266,1e-265,1e-264,
            1e-263,1e-262,1e-261,1e-260,1e-259,1e-258,1e-257,1e-256,1e-255,1e-254,
            1e-253,1e-252,1e-251,1e-250,1e-249,1e-248,1e-247,1e-246,1e-245,1e-244,
            1e-243,1e-242,1e-241,1e-240,1e-239,1e-238,1e-237,1e-236,1e-235,1e-234,
            1e-233,1e-232,1e-231,1e-230,1e-229,1e-228,1e-227,1e-226,1e-225,1e-224,
            1e-223,1e-222,1e-221,1e-220,1e-219,1e-218,1e-217,1e-216,1e-215,1e-214,
            1e-213,1e-212,1e-211,1e-210,1e-209,1e-208,1e-207,1e-206,1e-205,1e-204,
            1e-203,1e-202,1e-201,1e-200,1e-199,1e-198,1e-197,1e-196,1e-195,1e-194,
            1e-193,1e-192,1e-191,1e-190,1e-189,1e-188,1e-187,1e-186,1e-185,1e-184,
            1e-183,1e-182,1e-181,1e-180,1e-179,1e-178,1e-177,1e-176,1e-175,1e-174,
            1e-173,1e-172,1e-171,1e-170,1e-169,1e-168,1e-167,1e-166,1e-165,1e-164,
            1e-163,1e-162,1e-161,1e-160,1e-159,1e-158,1e-157,1e-156,1e-155,1e-154,
            1e-153,1e-152,1e-151,1e-150,1e-149,1e-148,1e-147,1e-146,1e-145,1e-144,
            1e-143,1e-142,1e-141,1e-140,1e-139,1e-138,1e-137,1e-136,1e-135,1e-134,
            1e-133,1e-132,1e-131,1e-130,1e-129,1e-128,1e-127,1e-126,1e-125,1e-124,
            1e-123,1e-122,1e-121,1e-120,1e-119,1e-118,1e-117,1e-116,1e-115,1e-114,
            1e-113,1e-112,1e-111,1e-110,1e-109,1e-108,1e-107,1e-106,1e-105,1e-104,
            1e-103,1e-102,1e-101,1e-100,1e-99,1e-98,1e-97,1e-96,1e-95,1e-94,1e-93,
            1e-92,1e-91,1e-90,1e-89,1e-88,1e-87,1e-86,1e-85,1e-84,1e-83,1e-82,1e-81,
            1e-80,1e-79,1e-78,1e-77,1e-76,1e-75,1e-74,1e-73,1e-72,1e-71,1e-70,1e-69,
            1e-68,1e-67,1e-66,1e-65,1e-64,1e-63,1e-62,1e-61,1e-60,1e-59,1e-58,1e-57,
            1e-56,1e-55,1e-54,1e-53,1e-52,1e-51,1e-50,1e-49,1e-48,1e-47,1e-46,1e-45,
            1e-44,1e-43,1e-42,1e-41,1e-40,1e-39,1e-38,1e-37,1e-36,1e-35,1e-34,1e-33,
            1e-32,1e-31,1e-30,1e-29,1e-28,1e-27,1e-26,1e-25,1e-24,1e-23,1e-22,1e-21,
            1e-20,1e-19,1e-18,1e-17,1e-16,1e-15,1e-14,1e-13,1e-12,1e-11,1e-10,1e-9,
            1e-8,1e-7,1e-6,1e-5,1e-4,1e-3,1e-2,1e-1,1e0,1e1,1e2,1e3,1e4,1e5,1e6,1e7,
            1e8,1e9,1e10,1e11,1e12,1e13,1e14,1e15,1e16,1e17,1e18,1e19,1e20,1e21,
            1e22,1e23,1e24,1e25,1e26,1e27,1e28,1e29,1e30,1e31,1e32,1e33,1e34,1e35,
            1e36,1e37,1e38,1e39,1e40,1e41,1e42,1e43,1e44,1e45,1e46,1e47,1e48,1e49,
            1e50,1e51,1e52,1e53,1e54,1e55,1e56,1e57,1e58,1e59,1e60,1e61,1e62,1e63,
            1e64,1e65,1e66,1e67,1e68,1e69,1e70,1e71,1e72,1e73,1e74,1e75,1e76,1e77,
            1e78,1e79,1e80,1e81,1e82,1e83,1e84,1e85,1e86,1e87,1e88,1e89,1e90,1e91,
            1e92,1e93,1e94,1e95,1e96,1e97,1e98,1e99,1e100,1e101,1e102,1e103,1e104,
            1e105,1e106,1e107,1e108,1e109,1e110,1e111,1e112,1e113,1e114,1e115,1e116,
            1e117,1e118,1e119,1e120,1e121,1e122,1e123,1e124,1e125,1e126,1e127,1e128,
            1e129,1e130,1e131,1e132,1e133,1e134,1e135,1e136,1e137,1e138,1e139,1e140,
            1e141,1e142,1e143,1e144,1e145,1e146,1e147,1e148,1e149,1e150,1e151,1e152,
            1e153,1e154,1e155,1e156,1e157,1e158,1e159,1e160,1e161,1e162,1e163,1e164,
            1e165,1e166,1e167,1e168,1e169,1e170,1e171,1e172,1e173,1e174,1e175,1e176,
            1e177,1e178,1e179,1e180,1e181,1e182,1e183,1e184,1e185,1e186,1e187,1e188,
            1e189,1e190,1e191,1e192,1e193,1e194,1e195,1e196,1e197,1e198,1e199,1e200,
            1e201,1e202,1e203,1e204,1e205,1e206,1e207,1e208,1e209,1e210,1e211,1e212,
            1e213,1e214,1e215,1e216,1e217,1e218,1e219,1e220,1e221,1e222,1e223,1e224,
            1e225,1e226,1e227,1e228,1e229,1e230,1e231,1e232,1e233,1e234,1e235,1e236,
            1e237,1e238,1e239,1e240,1e241,1e242,1e243,1e244,1e245,1e246,1e247,1e248,
            1e249,1e250,1e251,1e252,1e253,1e254,1e255,1e256,1e257,1e258,1e259,1e260,
            1e261,1e262,1e263,1e264,1e265,1e266,1e267,1e268,1e269,1e270,1e271,1e272,
            1e273,1e274,1e275,1e276,1e277,1e278,1e279,1e280,1e281,1e282,1e283,1e284,
            1e285,1e286,1e287,1e288,1e289,1e290,1e291,1e292,1e293,1e294,1e295,1e296,
            1e297,1e298,1e299,1e300,1e301,1e302,1e303,1e304,1e305,1e306,1e307,1e308
        };
        // clang-format on

        return constants[exponent + 323];
    }
};

template <typename Builder, bool UseCharconv>
class t_parser : public t_parser_base {
    const char* m_text_begin;
    const char* m_text_end;
    Builder& m_builder;

    // error handlng
    const char* m_error_location;
    errc m_error_code;
    std::string_view m_error_arg;

    const char* fail(const char* p, parse_error::errc code, std::string_view arg = {}) noexcept {
        m_error_location = p;
        m_error_code = code;
        m_error_arg = arg;
        return nullptr;
    }

    const char* skip_whitespace(const char* p) noexcept {
        while (true) {
            if (at_eof(p)) [[unlikely]] {
                return nullptr;
            }
            else if (is_whitespace(*p)) {
                ++p;
            }
            else {
                return p;
            }
        }
    }

    FORCE_INLINE bool at_eof(const char* p) const noexcept {
        return p == m_text_end;
    }
    FORCE_INLINE bool has_remaining_characters(const char* p, int n) const noexcept {
        return p + n <= m_text_end;
    }

    // custom number parser
    // faster than charconv for floating point numbers on MSSTL
    // parses all float32 numbers correctly, but drops precision for double numbers with many fractional digits
    const char* parse_number_custom(const char* p) {
        static constexpr auto int_max = std::numeric_limits<int32_t>::max();
        static constexpr auto int_min = std::numeric_limits<int32_t>::min();
        static constexpr auto risky = unsigned(int_max) / 10;

        unsigned max_digit_after_risky = unsigned(int_max) % 10;

        bool negative = false;
        if ('-' == *p) {
            ++p;
            negative = true;

            if (at_eof(p)) [[unlikely]] {
                return fail(p, errc::unexpected_end);
            }

            ++max_digit_after_risky;
        }

        bool try_double = false;

        unsigned u = 0;
        double d = 0.0; // gcc complains that d might be used uninitialized which isn't true. Fix the warning anyway.
        int64_t exponent = 0;

        if (*p == '0') {
            ++p;
            if (at_eof(p)) [[unlikely]] {
                goto done;
            }
        }
        else {
            unsigned char c = *p;
            if (c < '0' || c > '9') [[unlikely]] {
                return fail(p, errc::invalid_number);
            }

            do {
                unsigned char digit = c - '0';

                if (!try_double && (u > risky || (u == risky && digit > max_digit_after_risky))) [[unlikely]] {
                    // TODO: could split this into two loops
                    try_double = true;
                    d = u;
                }
                if (try_double) [[unlikely]] {
                    d = 10.0 * d + digit;
                }
                else {
                    u = 10 * u + digit;
                }

                ++p;
                if (at_eof(p)) [[unlikely]] {
                    goto done;
                }
                c = *p;
            } while (c >= '0' && c <= '9');
        }

        if ('.' == *p) {
            if (!try_double) {
                try_double = true;
                d = u;
            }
            ++p;
            if (at_eof(p)) [[unlikely]] {
                return fail(p, errc::unexpected_end);
            }
            char c = *p;
            if (c < '0' || c > '9') [[unlikely]] {
                return fail(p, errc::invalid_number);
            }

            do {
                d = d * 10 + (c - '0');
                // One option to avoid underflow would be to clamp to int_min, but int64 subtraction is cheap and
                // in the absurd case of parsing 2 GB of digits with an extremely high exponent, this will produce
                // accurate results.
                // Instead, we just leave exponent as int64_t and it will never underflow.
                --exponent;

                ++p;
                if (at_eof(p)) [[unlikely]] {
                    goto done;
                }

                c = *p;
            } while (c >= '0' && c <= '9');
        }

        if ('e' == *p || 'E' == *p) {
            if (!try_double) {
                try_double = true;
                d = u;
            }
            ++p;
            if (at_eof(p)) [[unlikely]] {
                return fail(p, errc::unexpected_end);
            }

            bool negative_exponent = false;
            if ('-' == *p) {
                negative_exponent = true;
                ++p;
                if (at_eof(p)) [[unlikely]] {
                    return fail(p, errc::unexpected_end);
                }
            }
            else if ('+' == *p) {
                ++p;
                if (at_eof(p)) [[unlikely]] {
                    return fail(p, errc::unexpected_end);
                }
            }

            int exp = 0;

            char c = *p;
            if (c < '0' || c > '9') [[unlikely]] {
                return fail(p, errc::invalid_number);
            }
            for (;;) {
                // c guaranteed to be between '0' and '9', inclusive
                unsigned char digit = c - '0';
                if (exp > (int_max - digit) / 10) {
                    // The exponent overflowed. Keep parsing, but it will definitely be out of range in the end.
                    exp = int_max;
                }
                else {
                    exp = 10 * exp + digit;
                }

                ++p;
                if (at_eof(p)) [[unlikely]] {
                    break;
                }

                c = *p;
                if (c < '0' || c > '9') {
                    break;
                }
            }
            static_assert(-int_max >= int_min, "exp can be negated without loss or UB");
            exponent += (negative_exponent ? -exp : exp);
        }

        done:

        if (exponent) {
            assert(try_double);
            // If d is zero but the exponent is huge, don't multiply zero by inf which gives nan.
            if (d != 0.0) {
                d *= pow10(exponent);
            }
        }

        if (negative) {
            if (try_double) {
                d = -d;
            }
            else {
                u = 0u - u;
            }
        }

        if (try_double) {
            m_builder.add_number_element(d);
        }
        else {
            m_builder.add_number_element(int32_t(u));
        }
        return p;
    }

    const char* parse_number_charconv(const char* p) {
        const auto begin = p;

        if ('-' == *p) {
            ++p;
            if (at_eof(p)) [[unlikely]] {
                return fail(p, errc::unexpected_end);
            }
        }

        bool match_double = false;
        while (*p >= '0' && *p <= '9') {
            ++p;
            if (at_eof(p)) [[unlikely]] {
                goto done_int;
            }
        }

        match_double = *p == '.' || *p == 'e' || *p == 'E';
        done_int:
        if (!match_double) {
            int64_t value = 0;
            auto res = POJOBUF_CHARCONV_NAMESPACE::from_chars(begin, p, value);
            if (res.ec == std::errc::result_out_of_range) {
                match_double = true;
            }
            else if (res.ec != std::errc()) {
                return fail(p, errc::invalid_number);
            }
            else {
                if (value >= std::numeric_limits<int32_t>::min() && value <= std::numeric_limits<int32_t>::max()) {
                    m_builder.add_number_element(int32_t(value));
                }
                else {
                    m_builder.add_number_element(int64_t(value));
                }
                return p;
            }
        }

        assert(match_double);
        double double_value = 0;
        auto res = POJOBUF_CHARCONV_NAMESPACE::from_chars(begin, m_text_end, double_value);
        if (res.ec != std::errc()) {
            return fail(p, errc::invalid_number);
        }
        m_builder.add_number_element(double_value);
        p = res.ptr;
        return p;
    }

    template <char... Args>
    const char* parse_literal(const char* p) {
        ++p;
        constexpr int length = sizeof...(Args);
        if (!has_remaining_characters(p, length)) [[unlikely]] {
            return fail(p, errc::unexpected_end);
        }
        constexpr char expected[] = {Args...};
        for (size_t i = 0; i < length; ++i) {
            if (p[i] != expected[i]) [[unlikely]] {
                return fail(p, errc::unexpected_, std::string_view(p + i, 1));
            }
        }
        return p + length;
    }

    const char* parse_string(const char* p, buffer_range& out_range) {
        ++p; // "
        const char* const begin = p;
        const char* input_end_local = m_text_end;
        // poor man's simd:
        while (input_end_local - p >= 4) {
            if (!is_plain_string_character(p[0])) {
                goto found;
            }
            if (!is_plain_string_character(p[1])) {
                p += 1;
                goto found;
            }
            if (!is_plain_string_character(p[2])) {
                p += 2;
                goto found;
            }
            if (!is_plain_string_character(p[3])) {
                p += 3;
                goto found;
            }
            p += 4;
        }
        for (;;) {
            if (p >= input_end_local) [[unlikely]] {
                return fail(p, errc::unexpected_end);
            }

            if (!is_plain_string_character(*p)) {
                break;
            }

            ++p;
        }
    found:
        if (*p == '"') [[likely]] {
            out_range = m_builder.push_internal_string(begin, p);
            return p + 1;
        }

        return parse_string_slow(p, out_range, begin);
    }

    const char* read_hex(const char* p, unsigned& u) {
        unsigned v = 0;
        int i = 4;
        while (i--) {
            unsigned char c = *p;
            if (c >= '0' && c <= '9') {
                c -= '0';
            }
            else if (c >= 'a' && c <= 'f') {
                c = c - 'a' + 10;
            }
            else if (c >= 'A' && c <= 'F') {
                c = c - 'A' + 10;
            }
            else {
                return fail(p, errc::invalid_, "utf8");
            }
            v = (v << 4) + c;
            ++p;
        }

        u = v;
        return p;
    }

    void write_utf8(unsigned codepoint, typename Builder::piecewise_string_builder& psb) {
        if (codepoint < 0x80) {
            psb.push(codepoint & 0xFF);
        }
        else if (codepoint < 0x800) {
            psb.push(0xC0 | ((codepoint >> 6) & 0xFF));
            psb.push(0x80 | (codepoint & 0x3F));
        }
        else if (codepoint < 0x10000) {
            psb.push(0xE0 | ((codepoint >> 12) & 0xFF));
            psb.push(0x80 | ((codepoint >> 6) & 0x3F));
            psb.push(0x80 | (codepoint & 0x3F));
        }
        else {
            assert(codepoint < 0x200000);
            psb.push(0xF0 | ((codepoint >> 18) & 0xFF));
            psb.push(0x80 | ((codepoint >> 12) & 0x3F));
            psb.push(0x80 | ((codepoint >> 6) & 0x3F));
            psb.push(0x80 | (codepoint & 0x3F));
        }
    }

    const char* parse_string_slow(const char* p, buffer_range& out_range, const char* const begin) {
        auto psb = m_builder.get_piecewise_string_builder(begin, p);
        const char* input_end_local = m_text_end;

        for (;;) {
            if (p >= input_end_local) [[unlikely]] {
                return fail(p, errc::unexpected_end);
            }

            if (*p >= 0 && *p < 0x20) [[unlikely]] {
                return fail(p, errc::illegal_codepoint);
            }

            switch (*p) {
            case '"':
                out_range = m_builder.push_string(psb);
                return p + 1;

            case '\\':
                ++p;
                if (p >= input_end_local) [[unlikely]] {
                    return fail(p, errc::unexpected_end);
                }

                char replacement;
                switch (*p) {
                case '"':
                    replacement = '"';
                    goto replace;
                case '\\':
                    replacement = '\\';
                    goto replace;
                case '/':
                    replacement = '/';
                    goto replace;
                case 'b':
                    replacement = '\b';
                    goto replace;
                case 'f':
                    replacement = '\f';
                    goto replace;
                case 'n':
                    replacement = '\n';
                    goto replace;
                case 'r':
                    replacement = '\r';
                    goto replace;
                case 't':
                    replacement = '\t';
                    goto replace;
                replace:
                    psb.push(replacement);
                    ++p;
                    break;
                case 'u': {
                    ++p;
                    if (!has_remaining_characters(p, 4)) [[unlikely]] {
                        return fail(p, errc::unexpected_end);
                    }
                    unsigned u = 0; // gcc's complaining that this could be used
                                    // uninitialized. wrong.
                    p = read_hex(p, u);
                    if (!p) [[unlikely]] {
                        return nullptr;
                    }
                    if (u >= 0xD800 && u <= 0xDBFF) {
                        if (!has_remaining_characters(p, 6)) [[unlikely]] {
                            return fail(p, errc::invalid_, "utf16");
                        }
                        char p0 = p[0];
                        char p1 = p[1];
                        if (p0 != '\\' || p1 != 'u') {
                            return fail(p, errc::expected_, "u");
                        }
                        p += 2;
                        unsigned v = 0; // gcc's complaining that this could be
                                        // used uninitialized. wrong.
                        p = read_hex(p, v);
                        if (!p) [[unlikely]] {
                            return nullptr;
                        }

                        if (v < 0xDC00 || v > 0xDFFF) {
                            return fail(p, errc::invalid_, "utf16");
                        }
                        u = 0x10000 + (((u - 0xD800) << 10) | (v - 0xDC00));
                    }
                    write_utf8(u, psb);
                    break;
                }
                default:
                    return fail(p, errc::unknown_escape);
                }
                break;

            default:
                // validate UTF-8
                unsigned char c0 = p[0];
                if (c0 < 128) {
                    psb.push(*p++);
                }
                else if (c0 < 224) {
                    if (!has_remaining_characters(p, 2)) [[unlikely]] {
                        return fail(p, errc::unexpected_end);
                    }
                    unsigned char c1 = p[1];
                    if (c1 < 128 || c1 >= 192) {
                        return fail(p + 1, errc::invalid_, "utf8");
                    }
                    psb.push(c0);
                    psb.push(c1);
                    p += 2;
                }
                else if (c0 < 240) {
                    if (!has_remaining_characters(p, 3)) [[unlikely]] {
                        return fail(p, errc::unexpected_end);
                    }
                    unsigned char c1 = p[1];
                    if (c1 < 128 || c1 >= 192) {
                        return fail(p + 1, errc::invalid_, "utf8");
                    }
                    unsigned char c2 = p[2];
                    if (c2 < 128 || c2 >= 192) {
                        return fail(p + 2, errc::invalid_, "utf8");
                    }
                    psb.push(c0);
                    psb.push(c1);
                    psb.push(c2);
                    p += 3;
                }
                else if (c0 < 248) {
                    if (!has_remaining_characters(p, 4)) [[unlikely]] {
                        return fail(p, errc::unexpected_end);
                    }
                    unsigned char c1 = p[1];
                    if (c1 < 128 || c1 >= 192) {
                        return fail(p + 1, errc::invalid_, "utf8");
                    }
                    unsigned char c2 = p[2];
                    if (c2 < 128 || c2 >= 192) {
                        return fail(p + 2, errc::invalid_, "utf8");
                    }
                    unsigned char c3 = p[3];
                    if (c3 < 128 || c3 >= 192) {
                        return fail(p + 3, errc::invalid_, "utf8");
                    }
                    psb.push(c0);
                    psb.push(c1);
                    psb.push(c2);
                    psb.push(c3);
                    p += 4;
                }
                else {
                    return fail(p, errc::invalid_, "utf8");
                }
                break;
            }
        }
    }

    const char* do_parse() {
        // init state machine
        auto p = m_text_begin;

        // state machine
        goto next_element;

        if (0) {
        empty_array_or_element:
            p = skip_whitespace(p + 1); // assume *p == '['
            if (!p) [[unlikely]] {
                return fail(p, errc::unexpected_end);
            }
            if (*p == ']') {
                goto pop_array;
            }
            else {
                goto next_element;
            }
            SPLAT_UNREACHABLE();

        empty_object_or_element:
            p = skip_whitespace(p + 1); // assume *p == '{'
            if (!p) [[unlikely]] {
                return fail(p, errc::unexpected_end);
            }
            if (*p == '}') {
                goto pop_object;
            }
            else {
                goto object_key;
            }
            SPLAT_UNREACHABLE();

        compound_close_or_comma:
            if (m_builder.current_compound_is_root()) [[unlikely]] {
                // end of json
                return p;
            }

            p = skip_whitespace(p);
            if (!p) [[unlikely]] {
                return fail(p, errc::unexpected_end);
            }

            if (m_builder.current_compound_is_array()) {
                if (*p == ']') {
                    goto pop_array;
                }
                else {
                    // expect comma
                    if (*p != ',') [[unlikely]] {
                        return fail(p, errc::expected_, ",");
                    }
                    ++p;
                    goto next_element;
                }
            }
            else {
                assert(m_builder.current_compound_is_object());
                if (*p == '}') {
                    goto pop_object;
                }
                else {
                    // expect comma
                    if (*p != ',') [[unlikely]] {
                        return fail(p, errc::expected_, ",");
                    }
                    ++p;
                    goto object_key;
                }
            }
            SPLAT_UNREACHABLE();

        pop_array:
            ++p; // skip ']'
            m_builder.template close_compound_element<pl_tag::array>();
            goto compound_close_or_comma;
            SPLAT_UNREACHABLE();

        pop_object:
            ++p; // skip '}'
            m_builder.template close_compound_element<pl_tag::object>();
            goto compound_close_or_comma;
            SPLAT_UNREACHABLE();

        object_key: {
            p = skip_whitespace(p);
            if (!p) [[unlikely]] {
                return fail(p, errc::unexpected_end);
            }
            if (*p != '"') [[unlikely]] {
                return fail(p, errc::missing_object_key);
            }
            buffer_range range;
            p = parse_string(p, range);
            if (!p) [[unlikely]] {
                return nullptr;
            }
            m_builder.add_object_key(range);
            p = skip_whitespace(p);
            if (!p) [[unlikely]] {
                return fail(p, errc::unexpected_end);
            }
            if (*p != ':') [[unlikely]] {
                return fail(p, errc::expected_, ":");
            }
            ++p; // skip colon
            goto next_element;
        }

        // read element at p
        next_element:
            p = skip_whitespace(p);
            if (!p) [[unlikely]] {
                return fail(p, errc::unexpected_end);
            }

            switch (*p) {
            case 'n':
                p = parse_literal<'u', 'l', 'l'>(p);
                if (!p) [[unlikely]] {
                    return nullptr;
                }
                m_builder.template add_literal_element<pl_tag::null>();
                break;
            case 'f':
                p = parse_literal<'a', 'l', 's', 'e'>(p);
                if (!p) [[unlikely]] {
                    return nullptr;
                }
                m_builder.template add_literal_element<pl_tag::false_>();
                break;
            case 't':
                p = parse_literal<'r', 'u', 'e'>(p);
                if (!p) [[unlikely]] {
                    return nullptr;
                }
                m_builder.template add_literal_element<pl_tag::true_>();
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '-': {
                if constexpr (UseCharconv) {
                    p = parse_number_charconv(p);
                }
                else {
                    p = parse_number_custom(p);
                }
                if (!p) [[unlikely]] {
                    return nullptr;
                }
                break;
            }
            case '"': {
                buffer_range range;
                p = parse_string(p, range);
                if (!p) [[unlikely]] {
                    return nullptr;
                }
                m_builder.add_string_element(range);
                break;
            }

            // for compound types make a payload that points to the current compound tag and base
            // so that we can backtrack appropriately
            // it will subsequently be overwritten with the actual computed payload of the compound type
            case '[': {
                m_builder.template open_compound_element<pl_tag::array>();
                goto empty_array_or_element;
            }
            case '{': {
                m_builder.template open_compound_element<pl_tag::object>();
                goto empty_object_or_element;
            }
            default:
                return fail(p, errc::unexpected_, std::string_view(p, 1));
            }

            goto compound_close_or_comma;
        }
        // end of state machine
        /////////////////////////////////////////

        SPLAT_UNREACHABLE();
    }
public:
    t_parser(std::string_view text, Builder& builder) noexcept
        : m_text_begin(text.data())
        , m_text_end(text.data() + text.size())
        , m_builder(builder)
    {}

    itlib::expected<const char*, parse_error> parse() {
        auto p = do_parse();
        if (!p) [[unlikely]] {
            return itlib::unexpected(parse_error::create(
                "pojobuf::json",
                m_error_code, std::string(m_error_arg),
                m_text_begin, m_error_location
            ));
        }
        return p;
    }
};

template <bool UseCharconv = false>
class parser {
public:
    static size_t get_buffer_size_for_text(size_t text_size) {
        return get_buffer_size_for_json(text_size);
    }
    static size_t get_buffer_size_for_text(std::string_view text) {
        return get_buffer_size_for_json(text);
    }
    static size_t get_scratch_buffer_size_for_text(size_t text_size) {
        return get_scratch_buffer_size_for_json(text_size);
    }
    static size_t get_scratch_buffer_size_for_text(std::string_view text) {
        return get_scratch_buffer_size_for_json(text.size());
    }

    template <typename Builder>
    static itlib::expected<const char*, parse_error> parse(std::string_view text, Builder& builder) {
        t_parser<Builder, UseCharconv> p(text, builder);
        return p.parse();
    }
};

using parser_charconv_num = parser<true>;
using parser_custom_num = parser<false>;

} // namespace pojobuf::json
