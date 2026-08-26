// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "document.hpp"
#include "docbuild.hpp"
#include "parse_error.hpp"
#include "parse_alloc_strategy.hpp"
#include <itlib/expected.hpp>

namespace pojobuf {

template <template <typename> class Parser, typename ByteBuf = no_buf>
itlib::expected<document<ByteBuf>, parse_error> document_parse(std::string_view source, size_t max_unsorted_obj_records = size_t(-1)) {
    using namespace docbuild;
    using data_alloc_type = single_buf_nocheck_data_alloc;
    using builder_type = buf_builder<data_alloc_type, valuebuf_byte_alloc<data_alloc_type>>;
    using parser_type = Parser<builder_type>;

    const auto buf_size = parser_type::get_buffer_size_for_text(source);
    bits::pod_vector buffer(buf_size);

    auto data_alloc = single_buf_nocheck_data_alloc::from_container(buffer);
    valuebuf_byte_alloc byte_alloc(data_alloc);
    builder_type builder(data_alloc, byte_alloc, max_unsorted_obj_records);

    Parser<builder_type> parser(source, builder);
    auto r = parser.parse();
    if (!r) {
        return itlib::unexpected(std::move(r).error());
    }
    buffer.resize(data_alloc.get_value_offset());

    return document<ByteBuf>(std::move(buffer), ByteBuf{}, nullptr, builder.finalize());
}

namespace bits {
struct deduce_t {};
}

template <template <typename> class Parser, typename DocByteBuf = bits::deduce_t, typename ArgByteBuf>
auto document_parse(parse_alloc_strategy strategy, ArgByteBuf&& source, size_t max_unsorted_obj_records = size_t(-1)) {
    using byte_buf_type = std::conditional_t<
        std::is_same_v<std::decay_t<DocByteBuf>, bits::deduce_t>,
        std::decay_t<ArgByteBuf>,
        DocByteBuf
    >;
    using ret_t = itlib::expected<document<byte_buf_type>, parse_error>;

    std::string_view source_sv(source.data(), source.size());
    if (strategy == parse_alloc_strategy::embed_bytes_in_data) {
        return document_parse<Parser, byte_buf_type>(source_sv, max_unsorted_obj_records);
    }

    using namespace docbuild;
    using data_alloc_type = single_buf_nocheck_data_alloc;
    using builder_type = buf_builder<data_alloc_type, mutable_source_byte_alloc>;
    using parser_type = Parser<builder_type>;

    const auto buf_size = parser_type::get_buffer_size_for_text(source_sv);
    bits::pod_vector buffer(buf_size);
    auto data_alloc = single_buf_nocheck_data_alloc::from_container(buffer);

    mutable_source_byte_alloc byte_alloc(source.data());
    builder_type builder(data_alloc, byte_alloc, max_unsorted_obj_records);

    Parser<builder_type> parser(source_sv, builder);

    auto r = parser.parse();
    if (!r) {
        return ret_t{itlib::unexpected(std::move(r).error())};
    }
    buffer.resize(data_alloc.get_value_offset());

    auto root_pl = builder.finalize();
    if (strategy == parse_alloc_strategy::take_source) {
        // we can take the source as is, no need to copy it
        return ret_t{document<byte_buf_type>(std::move(buffer), byte_buf_type{std::move(source)}, nullptr, root_pl)};
    }
    else if (strategy == parse_alloc_strategy::use_external_mutable_source) {
        return ret_t{document<byte_buf_type>(std::move(buffer), byte_buf_type{}, source.data(), root_pl)};
    }
    else {
        assert(false); // unknown strategy
        SPLAT_UNREACHABLE();
    }
}

} // namespace pojobuf
