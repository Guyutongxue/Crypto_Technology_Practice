#pragma once

#include <gmssl/sm2.h>
#include <gmssl/sm3.h>
#include <gmssl/sm4.h>

#include <cstdint>
#include <filesystem>
#include <ranges>
#include <type_traits>

#include "cli.hpp"
#include "fs.hpp"
#include "keygen.hpp"
#include "rand.hpp"
#include "util.hpp"

namespace detail {

constexpr const std::string_view DEFAULT_KEY_FILENAME{".encbox.key"};

inline std::string sm2Encrypt(const SM2_KEY& key, std::string_view plain) {
    assert(plain.size() <= SM2_MAX_PLAINTEXT_SIZE);
    auto plainPtr = reinterpret_cast<const std::uint8_t*>(plain.data());
    std::string cipher;
    cipher.resize_and_overwrite(
        SM2_MAX_CIPHERTEXT_SIZE,
        [&, sz = cipher.size()](char* buf, ...) mutable {
            if (sm2_encrypt(&key, plainPtr, plain.size(),
                            reinterpret_cast<std::uint8_t*>(buf), &sz) < 0) {
                throw std::runtime_error("failed to encrypt (sm2)");
            }
            return sz;
        });
    return cipher;
}
inline std::string sm2Decrypt(const SM2_KEY& key, std::string_view cipher) {
    assert(cipher.size() <= SM2_MAX_CIPHERTEXT_SIZE);
    auto cipherPtr = reinterpret_cast<const std::uint8_t*>(cipher.data());
    std::string plain;
    plain.resize_and_overwrite(
        SM2_MAX_PLAINTEXT_SIZE, [&, sz = plain.size()](char* buf, ...) mutable {
            if (sm2_decrypt(&key, cipherPtr, cipher.size(),
                            reinterpret_cast<std::uint8_t*>(buf), &sz) < 0) {
                throw std::runtime_error("failed to decrypt (sm2)");
            }
            return sz;
        });
    return plain;
}

using Sm4Key = const std::span<std::uint8_t, SM4_KEY_SIZE>;

inline std::string sm4Encrypt(Sm4Key key, const Sm4Key& iv,
                              std::string_view plain) {
    SM4_KEY sm4key;
    sm4_set_encrypt_key(&sm4key, key.data());

    std::string cipher;
    auto cipherSize = plain.size() + SM4_BLOCK_SIZE;
    cipher.resize_and_overwrite(cipherSize, [&](char* buf, ...) {
        if (sm4_cbc_padding_encrypt(
                &sm4key, iv.data(),
                reinterpret_cast<const std::uint8_t*>(plain.data()),
                plain.size(), reinterpret_cast<std::uint8_t*>(buf),
                &cipherSize) < 0) {
            throw std::runtime_error("failed to encrypt (sm4)");
        }
        return cipherSize;
    });
    return cipher;
}

inline std::string sm4Decrypt(Sm4Key key, const Sm4Key& iv,
                              std::string_view cipher) {
    SM4_KEY sm4key;
    sm4_set_decrypt_key(&sm4key, key.data());

    std::string plain;
    auto plainSize = cipher.size();
    plain.resize_and_overwrite(plainSize, [&](char* buf, ...) {
        if (sm4_cbc_padding_decrypt(
                &sm4key, iv.data(),
                reinterpret_cast<const std::uint8_t*>(cipher.data()),
                cipher.size(), reinterpret_cast<std::uint8_t*>(buf),
                &plainSize) < 0) {
            throw std::runtime_error("failed to decrypt (sm4)");
        }
        return plainSize;
    });
    return plain;
}

struct EncryptedFileHeader {
    char magic1[8]{'E', 'N', 'C', 'R', 'Y', 'P', 'T', '!'};
    std::uint8_t iv[SM4_BLOCK_SIZE];
    struct {
        std::uint8_t enableDigest;
        std::uint8_t reserve[3];
    } ctrl;
    static_assert(sizeof(ctrl) == 4);
    std::uint8_t digest[SM3_DIGEST_SIZE];
    std::uint8_t magic2[4]{0xba, 0xd1, 0xab, 0xel};

    static_assert(sizeof(std::uint8_t) == sizeof(char));
    bool checkMagic() {
        return std::memcmp(magic1, "ENCRYPT!", 8) == 0 &&
               std::memcmp(magic2, "\xba\xd1\xab\xel", 4) == 0;
    }
};
static_assert(std::is_trivially_copyable_v<EncryptedFileHeader>);

inline void doDecrypt(EncryptOption opt) {
    // get SM2 key to decrypt the symmetric key
    SM2_KEY sm2key;
    auto type = readKey(opt.pkey, sm2key);
    if (type != KeyType::Private) {
        throw std::runtime_error(std::format(
            "invalid key type: expected private key, got public key"));
    }

    auto inplace = std::filesystem::equivalent(opt.output, opt.input);
    if (!inplace && std::filesystem::exists(opt.output) && !opt.force) {
        throw std::runtime_error(std::format(
            "file {} already exists; use --force to overwrite", opt.output));
    }

    if (!std::filesystem::is_directory(opt.input)) {
        throw std::runtime_error(
            std::format("input {} is not a directory", opt.input));
    }
    // read symmetric key, decrypt it with SM2
    auto keyPath =
        std::move(opt.keyPath)
            .transform(
                [](auto&& s) { return std::filesystem::path{std::move(s)}; })
            .value_or(std::filesystem::path{opt.input} / DEFAULT_KEY_FILENAME);
    auto encryptedKey = readFile(keyPath);
    auto symKeyContent = sm2Decrypt(sm2key, encryptedKey);
    if (!symKeyContent.starts_with("encbox_key{") ||
        !symKeyContent.ends_with("}")) {
        throw std::runtime_error(
            "invalid key file (bad format); maybe .encbox.key file is "
            "corrupted");
    }
    auto symKey = base64Decode(
        std::string_view{symKeyContent}.substr(11, symKeyContent.size() - 12));
    if (symKey.size() != SM4_KEY_SIZE) {
        throw std::runtime_error(
            "invalid key file (wrong length); maybe .encbox.key file is "
            "corrupted");
    }
    auto keySpan =
        Sm4Key{reinterpret_cast<std::uint8_t*>(symKey.data()), symKey.size()};

    // remove key file temporarily
    std::filesystem::remove(keyPath);
    bool success = false;
    auto recoverKey = [&](int* ptr) {
        delete ptr;
        if (!(inplace && success)) {
            writeFile(keyPath, encryptedKey, true);
        }
    };
    auto recoverObj =
        std::unique_ptr<int, decltype(recoverKey)>{new int, recoverKey};

    // for each entry, decrypt it with SM4
    auto doForceWrite = inplace || opt.force;
    traverseDirectory(
        opt.input, opt.output, [&](std::string content, const auto& dest) {
            auto contentView = std::string_view{content};
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
            auto plain = sm4Decrypt(keySpan, efh.iv, cipher);
            // verify digest if necessary
            if (efh.ctrl.enableDigest && opt.enableDigest) {
                std::uint8_t encryptedDigest[SM3_DIGEST_SIZE];
                sm3_digest(reinterpret_cast<const std::uint8_t*>(plain.data()),
                           plain.size(), encryptedDigest);
                if (!std::ranges::equal(encryptedDigest, efh.digest)) {
                    throw std::runtime_error(
                        std::format("file {} is corrupted (digest verify fail)",
                                    dest.string()));
                }
            }
            writeFile(dest, plain, doForceWrite);
        });
    success = true;
}

inline void doEncrypt(EncryptOption opt) {
    // get SM2 key to encrypt the symmetric key
    SM2_KEY sm2key;
    auto type = readKey(opt.pkey, sm2key);
    if (type != KeyType::Public) {
        throw std::runtime_error(std::format(
            "invalid key type: expected public key, got private key"));
    }

    auto inplace = std::filesystem::equivalent(opt.output, opt.input);
    if (!inplace && std::filesystem::exists(opt.output) && !opt.force) {
        throw std::runtime_error(std::format(
            "file {} already exists; use --force to overwrite", opt.output));
    }
    // generate symmetric key, encrypt it with SM2
    if (!std::filesystem::is_directory(opt.input)) {
        throw std::runtime_error(
            std::format("input {} is not a directory", opt.input));
    }
    auto keyPath =
        std::move(opt.keyPath)
            .transform(
                [](auto&& s) { return std::filesystem::path{std::move(s)}; })
            .value_or(std::filesystem::path{opt.output} / DEFAULT_KEY_FILENAME);
    auto key = randomIntArray<std::uint8_t, SM4_KEY_SIZE>(*opt.encryptSeed);
    auto symKeyContent =
        std::format("encbox_key{{{}}}", base64Encode(spanToSv(std::span{key})));
    auto encryptedKey = sm2Encrypt(sm2key, symKeyContent);
    writeFile(keyPath, encryptedKey, opt.force);

    // for each entry, encrypt it with SM4
    auto doForceWrite = inplace || opt.force;
    traverseDirectory(
        opt.input, opt.output, [&](std::string content, const auto& dest) {
            // do not overwrite key
            if (std::filesystem::equivalent(keyPath, dest)) {
                return;
            }
            // assemble file header
            EncryptedFileHeader efh;
            auto iv = randomIntArray<std::uint8_t, 16>();
            std::ranges::copy(iv, efh.iv);
            efh.ctrl.enableDigest = opt.enableDigest;
            if (opt.enableDigest) {
                sm3_digest(
                    reinterpret_cast<const std::uint8_t*>(content.data()),
                    content.size(), efh.digest);
            }
            // encrypt with SM4
            auto cipher = sm4Encrypt(key, iv, content);
            auto ofs = writeFile(dest, doForceWrite);
            ofs.write(reinterpret_cast<const char*>(&efh), sizeof(efh));
            ofs.write(cipher.data(), cipher.size());
        });
}

}  // namespace detail

namespace encrypt {

inline void entry(EncryptOption opt) {
    return (opt.encryptSeed ? detail::doEncrypt
                            : detail::doDecrypt)(std::move(opt));
}

}  // namespace encrypt