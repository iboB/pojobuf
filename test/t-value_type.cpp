#include <pojobuf/value_type.hpp>
#include <doctest/doctest.h>

using pojobuf::value_type, pojobuf::value_tag;

constexpr value_type i32 = value_tag::int32;
static_assert(*i32 == value_tag::int32);
static_assert(i32 == value_type(value_tag::int32));
static_assert(i32.is_int32());
static_assert(i32.is_integer());
static_assert(i32.is_number());
static_assert(!i32.is_null());

constexpr value_type str = value_tag::string;
static_assert(*str == value_tag::string);
static_assert(str == value_type(value_tag::string));
static_assert(str.is_string());
static_assert(str.is_bytes());
static_assert(!str.is_blob());
static_assert(!str.is_number());
static_assert(!str.is_null());

constexpr value_type f = value_tag::false_;
static_assert(f.is_false());
static_assert(f.is_falsy());
static_assert(f.is_boolean());

constexpr value_type so = value_tag::sorted_object;
static_assert(so.is_sorted_object());
static_assert(so.is_object());
static_assert(so.is_compound());

static_assert(value_type(value_tag::undefined).is_falsy());
static_assert(value_type(value_tag::null).is_falsy());

static_assert(value_type(value_tag::real).is_number());

static_assert(value_type(value_tag::array).is_compound());
