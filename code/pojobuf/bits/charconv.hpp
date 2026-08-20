// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

#if POJOBUF_USE_MSCHARCONV
#   include <msstl/charconv.hpp>
#	define POJOBUF_CHARCONV_NAMESPACE msstl
#else
#   include <charconv>
#	define POJOBUF_CHARCONV_NAMESPACE std
#endif
