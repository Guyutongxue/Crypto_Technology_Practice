#pragma once

#include <openssl/evp.h>

#include "util.hpp"

namespace sm3 {

constexpr const std::size_t DIGEST_SIZE{32};

inline UString digest(const UString& data) {
    EVP_MD_CTX* ctx{};
    SSL_CHECK(ctx = EVP_MD_CTX_new());
    defer {
        EVP_MD_CTX_free(ctx);
    };
    SSL_CHECK(EVP_DigestInit_ex(ctx, EVP_sm3(), nullptr));
    SSL_CHECK(EVP_DigestUpdate(ctx, data.data(), data.size()));
    UString digest;
    digest.resize_and_overwrite(DIGEST_SIZE, [&](std::uint8_t* buf, ...) {
        SSL_CHECK(EVP_DigestFinal_ex(ctx, buf, nullptr));
        return DIGEST_SIZE;
    });
    return digest;
}

}  // namespace sm3
