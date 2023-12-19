#pragma once

#include <string>
#include <string_view>

#include "util.hpp"

constexpr const std::string_view base64Table{
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

inline std::string base64Encode(UStringView input) {
    std::string output;
    output.reserve((input.size() + 2) / 3 * 4);
    for (std::size_t i{0}; i < input.size(); i += 3) {
        std::size_t n{0};
        n |= static_cast<std::size_t>(input[i]) << 16;
        if (i + 1 < input.size()) {
            n |= static_cast<std::size_t>(input[i + 1]) << 8;
        }
        if (i + 2 < input.size()) {
            n |= static_cast<std::size_t>(input[i + 2]);
        }
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

inline UString base64Decode(std::string_view input) {
    UString output;
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
        output.push_back(static_cast<std::uint8_t>((n >> 16) & 0xff));
        if (input[i + 2] != '=') {
            output.push_back(static_cast<std::uint8_t>((n >> 8) & 0xff));
        }
        if (input[i + 3] != '=') {
            output.push_back(static_cast<std::uint8_t>(n & 0xff));
        }
    }
    return output;
}