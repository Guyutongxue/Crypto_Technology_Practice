#pragma once

#include <openssl/evp.h>

#include "util.hpp"

namespace sm4 {

constexpr const std::size_t KEY_SIZE{16};
constexpr const std::size_t BLOCK_SIZE{16};

inline UString encryptCbc(UStringView plain, const UString& key,
                          const UString& iv) {
    EVP_CIPHER_CTX* ctx{};
    SSL_CHECK(ctx = EVP_CIPHER_CTX_new());
    defer {
        EVP_CIPHER_CTX_free(ctx);
    };
    SSL_CHECK(
        EVP_EncryptInit_ex(ctx, EVP_sm4_cbc(), nullptr, key.data(), iv.data()));
    UString cipher;
    int cipherLen = plain.size() + 16;
    cipher.resize_and_overwrite(cipherLen, [&](std::uint8_t* buf, ...) {
        SSL_CHECK(EVP_EncryptUpdate(ctx, buf, &cipherLen, plain.data(),
                                plain.size()));
        return cipherLen;
    });
    int finalLen = 16;
    cipher.resize_and_overwrite(
        cipherLen + finalLen, [&](std::uint8_t* buf, ...) {
            SSL_CHECK(EVP_EncryptFinal_ex(ctx, buf + cipherLen, &finalLen));
            return cipherLen + finalLen;
        });
    return cipher;
}

inline UString decryptCbc(UStringView cipher, const UString& key,
                          const UString& iv) {
    EVP_CIPHER_CTX* ctx{};
    SSL_CHECK(ctx = EVP_CIPHER_CTX_new());
    defer {
        EVP_CIPHER_CTX_free(ctx);
    };
    SSL_CHECK(
        EVP_DecryptInit_ex(ctx, EVP_sm4_cbc(), nullptr, key.data(), iv.data()));
    UString plain;
    int plainLen = cipher.size();
    plain.resize_and_overwrite(plainLen, [&](std::uint8_t* buf, ...) {
        SSL_CHECK(EVP_DecryptUpdate(ctx, buf, &plainLen, cipher.data(),
                                cipher.size()));
        return plainLen;
    });
    int finalLen = 16;
    plain.resize_and_overwrite(
        plainLen + finalLen, [&](std::uint8_t* buf, ...) {
            SSL_CHECK(EVP_DecryptFinal_ex(ctx, buf + plainLen, &finalLen));
            return plainLen + finalLen;
        });
    return plain;
}

}  // namespace sm4