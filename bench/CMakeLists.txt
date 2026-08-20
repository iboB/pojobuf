# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/picobench@2.10.0)

include(icm_cpu_arch)
icm_get_native_arch_flag(nativeArchFlag)

macro(pojobuf_benchmark name)
    set(tgt bench-pojobuf-${name})
    add_executable(${tgt})
    set_target_properties(${tgt} PROPERTIES FOLDER bench)
    target_sources(${tgt} PRIVATE b-${name}.cpp ${ARGN})
    target_compile_options(${tgt} PRIVATE ${nativeArchFlag})
    target_link_libraries(${tgt} PRIVATE
        pojobuf::pojobuf
        picobench::picobench
    )
    add_custom_target(
        run-${tgt}
        COMMAND ${tgt}
    )
    set_target_properties(run-${tgt} PROPERTIES FOLDER bench)
endmacro()

