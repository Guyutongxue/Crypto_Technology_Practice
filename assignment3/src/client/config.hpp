#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <toml++/toml.hpp>

#include "sm2.hpp"
#include "toml++/impl/parser.hpp"

struct Config {
    std::string user;
    std::string hostname;
    std::uint16_t port;
};

namespace detail {

inline std::string readTextFile(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) {
        throw std::runtime_error("cannot open file " + path);
    }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

template <typename T>
    requires requires(T t, std::ostream os) { os << t; }
inline void writeTextFile(const std::string& path, T content) {
    std::ofstream ofs(path);
    if (!ofs) {
        throw std::runtime_error("cannot open file " + path);
    }
    ofs << content;
}

}  // namespace detail

constexpr const std::string DEFAULT_HOSTNAME = "localhost";
constexpr const std::uint16_t DEFAULT_PORT = 8080;

using namespace std::literals;

inline Config loadConfig(const std::string& path) {
    auto config = toml::parse_file(path);
    std::string user;
    if (auto got = config["user"].value<std::string>()) {
        user = *got;
    }
    auto hostname = config["server"]["hostname"].value_or(DEFAULT_HOSTNAME);
    auto port = config["server"]["port"].value_or(DEFAULT_PORT);
    return {
        .user = std::move(user), .hostname = std::move(hostname), .port = port};
}

constexpr const std::string DEFAULT_CONFIG_PATH = "config.toml";
