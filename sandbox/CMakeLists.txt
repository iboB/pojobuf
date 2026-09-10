# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_executable(pojobuf-sandbox
    sandbox.cpp
)
target_link_libraries(pojobuf-sandbox PRIVATE
    pojobuf::pojobuf
    sajson::sajson
    pojobuf::dev
    test-data::json
)
set_target_properties(pojobuf-sandbox PROPERTIES FOLDER sandbox)
