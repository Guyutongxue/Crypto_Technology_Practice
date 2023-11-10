#pragma once

#include <climits>
#include <cstring>
#include <format>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

constexpr const std::string_view base64Table{
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

static_assert(CHAR_BIT == 8);

inline std::string base64Encode(std::string_view input) {
    std::string output;
    output.reserve((input.size() + 2) / 3 * 4);
    for (std::size_t i{0}; i < input.size(); i += 3) {
        std::size_t n{0};
#define UNSIGNED_EXPAND(x) \
    (static_cast<std::size_t>(static_cast<unsigned char>(x)))
        n |= UNSIGNED_EXPAND(input[i]) << 16;
        if (i + 1 < input.size()) {
            n |= UNSIGNED_EXPAND(input[i + 1]) << 8;
        }
        if (i + 2 < input.size()) {
            n |= UNSIGNED_EXPAND(input[i + 2]);
        }
#undef UNSIGNED_EXPAND
        output.push_back(base64Table[(n >> 18) & 0x3f]);
        output.push_back(base64Table[(n >> 12) & 0x3f]);
        if (i + 1 < input.size()) {
            output.push_back(base64Table[(n >> 6) & 0x3f]);
        } else {
            output.push_back('=');
        }
        if (i + 2 < input.size()) {
            output.push_back(base64Table[n & 0x3f]);
        } else {
            output.push_back('=');
        }
    }
    return output;
}

inline std::string base64Decode(std::string_view input) {
    std::string output;
    output.reserve(input.size() / 4 * 3);
    for (std::size_t i{0}; i < input.size(); i += 4) {
        std::size_t n{0};
        n |= base64Table.find(input[i]) << 18;
        n |= base64Table.find(input[i + 1]) << 12;
        if (input[i + 2] != '=') {
            n |= base64Table.find(input[i + 2]) << 6;
        }
        if (input[i + 3] != '=') {
            n |= base64Table.find(input[i + 3]);
        }
        output.push_back(static_cast<char>((n >> 16) & 0xff));
        if (input[i + 2] != '=') {
            output.push_back(static_cast<char>((n >> 8) & 0xff));
        }
        if (input[i + 3] != '=') {
            output.push_back(static_cast<char>(n & 0xff));
        }
    }
    return output;
}

template <std::integral T, std::size_t N = std::dynamic_extent>
inline std::string_view spanToSv(const std::span<T, N> src) {
    return std::string_view(reinterpret_cast<const char*>(src.data()),
                            src.size());
}

template <typename T>
    requires(std::is_trivially_copyable_v<T>)
inline std::string_view podToSv(const T& src) {
    return std::string_view(reinterpret_cast<const char*>(&src), sizeof(src));
}

template <typename T>
    requires(std::is_trivially_copyable_v<T>)
inline void svToPod(std::string_view sv, T& target) {
    if (sv.size() != sizeof(T)) {
        throw std::runtime_error(
            std::format("invalid size of string_view: expected {}, got {}",
                        sizeof(T), sv.size()));
    }
    std::memcpy(&target, sv.data(), sizeof(T));
}

// #include <iostream>
// template <std::ranges::range R>
// inline void printRange(const R& range) {
//     for (auto&& e : range) {
//         std::cout << std::format("{:02x}", static_cast<unsigned char>(e));
//     }
//     std::cout << std::endl;
// }