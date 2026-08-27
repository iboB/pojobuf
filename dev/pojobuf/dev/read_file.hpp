// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

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

inline std::vector<std::string> read_lines(const std::string& path) {
    std::vector<std::string> ret;
    std::ifstream list(path);
    while (list) {
        std::string line;
        std::getline(list, line);
        if (!line.empty()) {
            ret.push_back(line);
        }
    }
    return ret;
}

} // namespace pojobuf::dev