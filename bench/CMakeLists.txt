# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/picobench@2.10.0)

include(icm_cpu_arch)
icm_get_native_arch_flag(nativeArchFlag)

# benchmarking against
CPMAddPackage("gh:iboB/sajson@1.0.1ibob#eec5b8e")
CPMAddPackage(gh:simdjson/simdjson@4.6.9)
if(NOT TARGET Boost::boost)
    CPMAddPackage(
        NAME Boost
        VERSION 1.87.0
        URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.xz
        URL_HASH SHA256=7da75f171837577a52bbf217e17f8ea576c7c246e4594d617bfde7fafd408be5
        SYSTEM TRUE
        OPTIONS "BOOST_ENABLE_CMAKE ON"
                "BOOST_INCLUDE_LIBRARIES json"
    )
endif()
if(NOT TARGET Boost::json)
    add_library(Boost::json ALIAS Boost::boost)
endif()

macro(pojobuf_benchmark name)
    set(tgt bench-pojobuf-${name})
    add_executable(${tgt})
    set_target_properties(${tgt} PROPERTIES FOLDER bench)
    target_sources(${tgt} PRIVATE b-${name}.cpp ${ARGN})
    target_compile_options(${tgt} PRIVATE ${nativeArchFlag})
    target_link_libraries(${tgt} PRIVATE
        picobench::picobench
        pojobuf::pojobuf
        sajson::sajson
        simdjson::simdjson
        Boost::json
        pojobuf::dev
        test-data::json
    )
    add_custom_target(
        run-${tgt}
        COMMAND ${tgt}
    )
    set_target_properties(run-${tgt} PROPERTIES FOLDER bench)
endmacro()

pojobuf_benchmark(json-parse)
pojobuf_benchmark(traverse)
