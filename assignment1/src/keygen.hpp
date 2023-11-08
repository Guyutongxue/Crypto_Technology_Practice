#pragma once

#include <gmssl/sm2.h>

#include <ranges>

#include "cli.hpp"
#include "fs.hpp"
#include "util.hpp"

inline void keygen(KeygenOption opt) {
    SM2_KEY sm2key;
    if (sm2_key_generate(&sm2key) < 0) {
        throw std::runtime_error("failed to generate key");
    }

    auto pubKeyContent = std::format(
        "----- BEGIN PUBLIC KEY -----\n{}\n----- END PUBLIC KEY -----\n",
        base64Encode(podToSv(sm2key.public_key)));
    auto privKeyContent = std::format(
        "----- BEGIN PRIVATE KEY -----\n{}\n----- END PRIVATE KEY -----\n",
        base64Encode(podToSv(sm2key.private_key)));

    writeFile(opt.output + ".pub", pubKeyContent, opt.force);
    writeFile(opt.output, privKeyContent, opt.force);
}

enum class KeyType { Public, Private };

inline KeyType readKey(const std::filesystem::path& path, SM2_KEY& sm2key) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error(
            std::format("cannot open file {}", path.c_str()));
    }
    std::vector<std::string> lines;
    while (ifs) {
        std::string line;
        std::getline(ifs, line);
        if (!line.empty()) {
            lines.push_back(std::move(line));
        }
    }
    if (lines.size() != 3) {
        throw std::runtime_error(std::format(
            "invalid key file {} (too many or less lines)", path.c_str()));
    }
    if (!(lines[0].starts_with("----- BEGIN ") &&
          lines[0].ends_with(" KEY -----") &&
          lines[2].starts_with("----- END ") &&
          lines[2].ends_with(" KEY -----"))) {
        throw std::runtime_error(
            std::format("invalid key file {} (bad format)", path.c_str()));
    }
    auto type = std::string_view(lines[0]).substr(12, lines[0].size() - 22);
    auto key = base64Decode(lines[1]);
    if (type == "PUBLIC") {
        if (key.size() != sizeof(sm2key.public_key)) {
            throw std::runtime_error(
                std::format("invalid key file {} (pubkey size)", path.c_str()));
        }
        svToPod(key, sm2key.public_key);
        return KeyType::Public;
    } else if (type == "PRIVATE") {
        if (key.size() != sizeof(sm2key.private_key)) {
            throw std::runtime_error(std::format(
                "invalid key file {} (privkey size)", path.c_str()));
        }
        svToPod(key, sm2key.private_key);
        return KeyType::Private;
    } else {
        throw std::runtime_error(std::format(
            "invalid key file {} (unknown key type)", path.c_str()));
    }
}