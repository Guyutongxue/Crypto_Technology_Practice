#pragma once

#include <cstring>
#include <random>

#include "cli.hpp"
#include "fs.hpp"
#include "sm2.hpp"
#include "sm3.hpp"
#include "sm4.hpp"
#include "util.hpp"
#include "web.hpp"

struct EncryptedFileHeader {
    char magic1[8]{'E', 'N', 'C', 'R', 'Y', 'P', 'T', '!'};
    std::uint8_t iv[sm4::BLOCK_SIZE];
    std::size_t encryptedKeyLength;
    std::uint8_t encryptedKey[220];
    std::uint8_t magic2[4]{0xba, 0xd1, 0xab, 0xel};

    static_assert(sizeof(std::uint8_t) == sizeof(char));
    bool checkMagic() const {
        return std::memcmp(magic1, "ENCRYPT!", 8) == 0 &&
               std::memcmp(magic2, "\xba\xd1\xab\xel", 4) == 0;
    }
};
static_assert(std::is_trivially_copyable_v<EncryptedFileHeader>);
static_assert(sizeof(EncryptedFileHeader) == 256);

template <typename T>
    requires(std::is_trivially_copyable_v<T>)
inline UStringView podToSv(const T& src) {
    return UStringView(reinterpret_cast<const std::uint8_t*>(&src),
                       sizeof(src));
}

template <typename T>
    requires(std::is_trivially_copyable_v<T>)
inline void svToPod(UStringView sv, T& target) {
    if (sv.size() != sizeof(T)) {
        throw std::runtime_error(
            std::format("invalid size of string_view: expected {}, got {}",
                        sizeof(T), sv.size()));
    }
    std::memcpy(&target, sv.data(), sizeof(T));
}

inline std::uint8_t randomChar() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<unsigned> dis(0, 1 << CHAR_BIT);
    return dis(gen);
}

inline void doEncrypt(EncryptOption opt) {
    auto content = readFile(opt.input);

    // assemble file header
    EncryptedFileHeader efh;
    std::ranges::generate(efh.iv, randomChar);

    // generate random sm4 key
    std::uint8_t key[sm4::KEY_SIZE];
    std::ranges::generate(key, randomChar);

    // get public key from server
    auto publicKey = getPublicKeyFromServer(opt.config);

    auto encryptedKey = sm2::encrypt(key, publicKey);
    efh.encryptedKeyLength = encryptedKey.size();
    std::memcpy(efh.encryptedKey, encryptedKey.data(), encryptedKey.size());

    // encrypt with SM4
    auto cipher = sm4::encryptCbc(content, key, efh.iv);
    UString final{podToSv(efh)};
    final += cipher;
    writeFile(opt.output, final, opt.force);
}

inline void doDecrypt(DecryptOption opt) {
    auto content = readFile(opt.input);
    auto contentView = UStringView{content};

    // Recreate header from raw bytes
    if (contentView.size() < sizeof(EncryptedFileHeader)) {
        throw std::runtime_error(
            std::format("invalid encrypted file header: too small"));
    }

    EncryptedFileHeader efh;
    svToPod(contentView.substr(0, sizeof(EncryptedFileHeader)), efh);
    // extract header info
    if (!efh.checkMagic()) {
        throw std::runtime_error(
            std::format("invalid encrypted file header; file maybe "
                        "corrupted or not encrypted before"));
    }

    // decrypt with sm4
    auto cipher = contentView.substr(sizeof(EncryptedFileHeader));
    auto key = decryptFromServer(
        opt.config, UStringView{efh.encryptedKey, efh.encryptedKeyLength});
    auto plain = sm4::decryptCbc(cipher, key, efh.iv);
    writeFile(opt.output, plain, opt.force);
}

namespace action {
inline void run(EncryptOption opt) {
    doEncrypt(opt);
}
inline void run(DecryptOption opt) {
    doDecrypt(opt);
}
}  // namespace action