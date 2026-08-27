// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <filesystem>
#include <fstream>

namespace pojobuf::dev {

template <typename Out = std::string>
Out read_file(const std::string& path) {
    const auto size = std::filesystem::file_size(path);
    Out ret;
    ret.resize(size);
    std::ifstream in(path, std::ios::binary);
    in.read(ret.data(), size);
    return ret;
}

} // namespace pojobuf::dev