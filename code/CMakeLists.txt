# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_library(pojobuf INTERFACE)
add_library(pojobuf::pojobuf ALIAS pojobuf)

CPMAddPackage(gh:iboB/splat@1.3.5)
CPMAddPackage(gh:iboB/itlib@1.12.3)
target_link_libraries(pojobuf INTERFACE
    splat::splat
    itlib::itlib
)

target_sources(pojobuf
    INTERFACE FILE_SET HEADERS FILES
        pojobuf/value_type.hpp
)

include(icm_check_charconv_fp_to_chars)
if(NOT haveCharconvFpToChars)
    message("pojobuf: no charconv floating point support detected. Using mscharconv")
    CPMAddPackage(gh:iboB/mscharconv@1.2.3)
    target_link_libraries(pojobuf PUBLIC msstl::charconv)
    target_compile_definitions(pojobuf PUBLIC -DHUSE_USE_MSCHARCONV=1)
else()
    message("pojobuf: detected charconv floating point support")
endif()
