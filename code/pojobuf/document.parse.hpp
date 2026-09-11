// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "document.hpp"
#include "docbuild.hpp"
#include "parse_error.hpp"
#include "parse_alloc_strategy.hpp"
#include "bits/deduce_t.hpp"
#include <itlib/expected.hpp>

namespace pojobuf {

template <
    typename Parser,
    parse_alloc_strategy Strategy,
    typename DocByteBuf = bits::deduce_t,
    typename Source
>
auto document_parse_with(Source&& source, size_t max_unsorted_obj_records = size_t(-1)) {
    using namespace docbuild;

    using byte_buf_type = std::conditional_t<
        std::is_same_v<std::decay_t<DocByteBuf>, bits::deduce_t>,
        std::decay_t<Source>,
        DocByteBuf
    >;
    using ret_t = itlib::expected<document<byte_buf_type>, parse_error>;

    std::string_view source_sv(std::data(source), std::size(source));

    const auto buf_size = Parser::get_buffer_size_for_text(source_sv);
    bits::pod_vector buffer(buf_size);

    auto data_alloc = single_buf_nocheck_data_alloc::from_container(buffer);
    auto byte_alloc = [&]() {
        if constexpr (Strategy == parse_alloc_strategy::embed_bytes_in_data) {
            return valuebuf_byte_alloc(data_alloc);
        }
        else {
            return mutable_source_byte_alloc(source.data());
        }
    }();

    buf_builder builder(data_alloc, byte_alloc, max_unsorted_obj_records);

    auto r = Parser::parse(source_sv, builder);
    if (!r) {
        return ret_t{itlib::unexpected(std::move(r).error())};
    }
    buffer.resize(data_alloc.get_value_offset());

    auto root_pl = builder.finalize();
    if constexpr (Strategy == parse_alloc_strategy::embed_bytes_in_data) {
        return ret_t{document<byte_buf_type>(std::move(buffer), byte_buf_type{}, nullptr, root_pl)};
    }
    else if constexpr (Strategy == parse_alloc_strategy::take_source) {
        // we can take the source as is, no need to copy it
        return ret_t{document<byte_buf_type>(std::move(buffer), byte_buf_type{std::forward<Source>(source)}, nullptr, root_pl)};
    }
    else if constexpr (Strategy == parse_alloc_strategy::use_external_mutable_source) {
        return ret_t{document<byte_buf_type>(std::move(buffer), byte_buf_type{}, std::data(source), root_pl)};
    }
    else {
        static_assert(Strategy == parse_alloc_strategy::embed_bytes_in_data, "Unsupported parse_alloc_strategy");
    }
}

template <typename Parser, typename ByteBuf = no_buf>
itlib::expected<document<ByteBuf>, parse_error> document_parse(
    std::string_view source,
    size_t max_unsorted_obj_records = size_t(-1)
) {
    return document_parse_with<
        Parser,
        parse_alloc_strategy::embed_bytes_in_data,
        ByteBuf
    >(source, max_unsorted_obj_records);
}

} // namespace pojobuf
