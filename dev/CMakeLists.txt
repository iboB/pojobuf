# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_library(pojobuf-dev INTERFACE)
add_library(pojobuf::dev ALIAS pojobuf-dev)
target_link_libraries(pojobuf-dev INTERFACE
    pojobuf::pojobuf
    splat::splat
    itlib::itlib
)
target_include_directories(pojobuf-dev INTERFACE .)

# dev deps for benchmarks and tests
CPMAddPackage(gh:iboB/json-test-data@1.1.0)

# used as oracle in json tests
CPMAddPackage(gh:nlohmann/json@3.12.0)
