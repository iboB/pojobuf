// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "document.hpp"
#include "docbuild.hpp"
#include "parse_error.hpp"
#include <itlib/expected.hpp>

namespace pojobuf {

template <template <typename> class Parser>
itlib::expected<document, parse_error> document_parse(std::string_view text, size_t max_unsorted_obj_records = size_t(0) - 1) {
    using namespace docbuild;
    using data_alloc_type = single_buf_nocheck_data_alloc;
    using builder_type = buf_builder<data_alloc_type, valuebuf_byte_alloc<data_alloc_type>>;
    using parser_type = Parser<builder_type>;

    const auto buf_size = parser_type::get_buffer_size_for_text(text);
    bits::pod_vector buffer(buf_size);

    auto data_alloc = single_buf_nocheck_data_alloc::from_container(buffer);
    valuebuf_byte_alloc byte_alloc(data_alloc);
    builder_type builder(data_alloc, byte_alloc, max_unsorted_obj_records);

    Parser<decltype(builder)> parser(text, builder);
    auto r = parser.parse();
    if (!r) {
        return itlib::unexpected(std::move(r).error());
    }
    buffer.resize(data_alloc.get_value_offset());

    return document(std::move(buffer), builder.finalize());
}

} // namespace pojobuf
