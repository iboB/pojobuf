# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/doctest-util@0.1.4)
set_target_properties(doctest PROPERTIES FOLDER test)
set_target_properties(doctest-main PROPERTIES FOLDER test)

macro(pojobuf_test test)
    add_doctest_lib_test(${test} pojobuf t-${test}.cpp ${ARGN})
endmacro()

pojobuf_test(value_type)
pojobuf_test(json LIBRARIES nlohmann_json::nlohmann_json test-data::json pojobuf::dev)
pojobuf_test(value)
pojobuf_test(document)
