# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
macro(pojobuf_example name)
    set(tgt pojobuf-example-${name})
    add_executable(${tgt})
    set_target_properties(${tgt} PROPERTIES FOLDER example)
    target_sources(${tgt} PRIVATE e-${name}.cpp ${ARGN})
    target_link_libraries(${tgt} PRIVATE
        pojobuf::pojobuf
    )
endmacro()

pojobuf_example(hello-json)
