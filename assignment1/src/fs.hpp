#pragma once

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>

inline std::ofstream writeFile(const std::filesystem::path& path, bool force) {
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
    return ofs;
}

inline void writeFile(const std::filesystem::path& path,
                      std::string_view content, bool force) {
    auto ofs = writeFile(path, force);
    ofs.write(content.data(), content.size());
}

inline std::string readFile(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error(
            std::format("file {} does not exist", path.string()));
    }
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error(
            std::format("cannot open file {}", path.string()));
    }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

template <typename F>
    requires std::invocable<F, std::string, const std::filesystem::path&>
inline void traverseDirectory(const std::filesystem::path& src,
                              const std::filesystem::path& dest, F&& f) {
    namespace fs = std::filesystem;
    if (!fs::exists(src)) {
        throw std::runtime_error(
            std::format("file {} does not exist", src.string()));
    }
    if (fs::is_symlink(src)) {
        std::cerr << std::format("Warning: skip symlink file {}\n",
                                 src.string())
                  << std::endl;
    } else if (fs::is_directory(src)) {
        for (auto& entry : fs::directory_iterator(src)) {
            traverseDirectory(entry.path(), dest / entry.path().filename(),
                              std::forward<F>(f));
        }
    } else if (fs::is_regular_file(src)) {
        f(readFile(src), dest);
    } else {
        throw std::runtime_error(
            std::format("file {} is not a regular file", src.string()));
    }
}
