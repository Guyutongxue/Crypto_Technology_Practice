#pragma once

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

#include "util.hpp"

inline void writeFile(const std::filesystem::path& path, UStringView content,
                      bool force) {
    namespace fs = std::filesystem;
    if (fs::exists(path) && !force) {
        throw std::runtime_error(std::format(
            "file {} already exists; use --force to overwrite", path.string()));
    }
    fs::create_directories(fs::absolute(path).parent_path());
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error(
            std::format("cannot open file {}", path.string()));
    }
    ofs.write(reinterpret_cast<const char*>(content.data()), content.size());
}

inline UString readFile(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error(
            std::format("file {} does not exist", path.string()));
    }
    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    if (!ifs) {
        throw std::runtime_error(
            std::format("cannot open file {}", path.string()));
    }
    std::size_t size = ifs.tellg();
    ifs.seekg(ifs.beg);
    UString result;
    result.resize_and_overwrite(size, [&](std::uint8_t* buf, ...) {
        ifs.read(reinterpret_cast<char*>(buf), size);
        return size;
    });
    return result;
}