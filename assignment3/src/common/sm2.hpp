#pragma once

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include "util.hpp"

namespace sm2 {

struct Pkey {
    std::string publicKey;
    std::string privateKey;
};

namespace detail {

class MemBio {
private:
    BIO* bio;

public:
    MemBio() {
        SSL_CHECK(bio = BIO_new(BIO_s_mem()));
    }
    ~MemBio() {
        BIO_free(bio);
    }
    operator BIO*() {
        return bio;
    }
    std::string content() {
        BUF_MEM* buf{};
        BIO_get_mem_ptr(bio, &buf);
        return std::string(buf->data, buf->data + buf->length);
    }
};

}  // namespace detail

inline Pkey generateKey() {
    EVP_PKEY_CTX* pctx{};
    SSL_CHECK(pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_SM2, nullptr));
    defer {
        EVP_PKEY_CTX_free(pctx);
    };
    SSL_CHECK(EVP_PKEY_keygen_init(pctx));
    EVP_PKEY* pkey{};
    SSL_CHECK(EVP_PKEY_keygen(pctx, &pkey));
    defer {
        EVP_PKEY_free(pkey);
    };

    detail::MemBio publicBio;
    SSL_CHECK(PEM_write_bio_PUBKEY(publicBio, pkey));
    detail::MemBio privateBio;
    SSL_CHECK(PEM_write_bio_PrivateKey(privateBio, pkey, nullptr, nullptr, 0,
                                   nullptr, nullptr));

    return {
        .publicKey = publicBio.content(),
        .privateKey = privateBio.content(),
    };
}

inline UString encrypt(UStringView plain, std::string pubPem) {
    detail::MemBio bio;
    SSL_CHECK(BIO_write(bio, pubPem.data(), pubPem.size()));
    EVP_PKEY* pkey{};
    SSL_CHECK(PEM_read_bio_PUBKEY(bio, &pkey, nullptr, nullptr));
    defer {
        EVP_PKEY_free(pkey);
    };

    EVP_PKEY_CTX* pctx{};
    SSL_CHECK(pctx = EVP_PKEY_CTX_new(pkey, nullptr));
    defer {
        EVP_PKEY_CTX_free(pctx);
    };
    SSL_CHECK(EVP_PKEY_encrypt_init(pctx));

    UString cipher;
    std::size_t cipherLen = plain.size() + 256;
    cipher.resize_and_overwrite(cipherLen, [&](std::uint8_t* buf, ...) {
        SSL_CHECK(EVP_PKEY_encrypt(pctx, cipher.data(), &cipherLen, plain.data(),
                               plain.size()));
        return cipherLen;
    });
    return cipher;
}

inline UString decrypt(UStringView cipher, std::string privPem) {
    detail::MemBio bio;
    SSL_CHECK(BIO_write(bio, privPem.data(), privPem.size()));
    EVP_PKEY* pkey{};
    SSL_CHECK(PEM_read_bio_PrivateKey(bio, &pkey, nullptr, nullptr));
    defer {
        EVP_PKEY_free(pkey);
    };

    EVP_PKEY_CTX* pctx{};
    SSL_CHECK(pctx = EVP_PKEY_CTX_new(pkey, nullptr));
    defer {
        EVP_PKEY_CTX_free(pctx);
    };
    SSL_CHECK(EVP_PKEY_decrypt_init(pctx));

    UString plain;
    std::size_t plainLen = cipher.size();
    plain.resize_and_overwrite(plainLen, [&](std::uint8_t* buf, ...) {
        SSL_CHECK(EVP_PKEY_decrypt(pctx, plain.data(), &plainLen, cipher.data(),
                               cipher.size()));
        return plainLen;
    });
    return plain;
}

}  // namespace sm2