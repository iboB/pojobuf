#include <pojobuf/value_type.hpp>
#include <doctest/doctest.h>

constexpr pojobuf::value_type v = pojobuf::pl_tag::int32;
static_assert(*v == pojobuf::pl_tag::int32);
static_assert(v == pojobuf::value_type(pojobuf::pl_tag::int32));
static_assert(v.is_int32());
static_assert(v.is_integer());
static_assert(v.is_number());
static_assert(!v.is_null());
