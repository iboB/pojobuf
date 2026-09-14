// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

namespace pojobuf {

enum class parse_alloc_strategy {
    use_external_mutable_source,
    embed_bytes_in_data,
    take_source, // ... which should lead to an hidden internal mutable source
};

} // namespace pojobuf
