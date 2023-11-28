// Copyright (C) 2023 Guyutongxue
//
// This file is part of assignment2.
//
// assignment2 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// assignment2 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with assignment2.  If not, see <http://www.gnu.org/licenses/>.

#include "cert.h"

#include <gmssl/sm2.h>
#include <gmssl/sm3.h>

#include <algorithm>
#include <cstring>
#include <format>
#include <iostream>
#include <stdexcept>

#include "Certificate.h"

using namespace std::literals;

namespace {

const int sm2WithSm3Oid[]{1, 2, 156, 10197, 1, 501};
const int ecPublicKeyOid[]{1, 2, 840, 10045, 2, 1};
const int sm2EccOid[]{1, 2, 156, 10197, 1, 301};
const int countryNameOid[]{2, 5, 4, 6};
const int organizationNameOid[]{2, 5, 4, 10};
const int organizationalUnitNameOid[]{2, 5, 4, 11};
const int commonNameOid[]{2, 5, 4, 3};

#define OBJECT_IDENTIFIER_set(target, arr) \
    OBJECT_IDENTIFIER_set_arcs(target, arr, sizeof(arr[0]), std::size(arr))

template <typename T>
T* typedMalloc() {
    return reinterpret_cast<T*>(std::calloc(sizeof(T), 1));
}

Name createName(const CertInfo& info) {
    Name name{
        .present = Name_PR_rdnSequence,
    };
    auto& rdnSeq = name.choice.rdnSequence;

    auto ptr = typedMalloc<RelativeDistinguishedName>();
    auto ptr2 = typedMalloc<AttributeTypeAndValue>();
    OBJECT_IDENTIFIER_set(&ptr2->type, countryNameOid);
    ptr2->value.present = AttributeValue_PR_printableString;
    OCTET_STRING_fromBuf(&ptr2->value.choice.printableString,
                         info.country.data(), info.country.size());
    ASN_SEQUENCE_ADD(&ptr->list, ptr2);
    ASN_SEQUENCE_ADD(&rdnSeq.list, ptr);

    ptr = typedMalloc<RelativeDistinguishedName>();
    ptr2 = typedMalloc<AttributeTypeAndValue>();
    OBJECT_IDENTIFIER_set(&ptr2->type, organizationNameOid);
    ptr2->value.present = AttributeValue_PR_utf8String;
    OCTET_STRING_fromBuf(&ptr2->value.choice.utf8String,
                         info.organization.data(), info.organization.size());
    ASN_SEQUENCE_ADD(&ptr->list, ptr2);
    ASN_SEQUENCE_ADD(&rdnSeq.list, ptr);

    ptr = typedMalloc<RelativeDistinguishedName>();
    ptr2 = typedMalloc<AttributeTypeAndValue>();
    OBJECT_IDENTIFIER_set(&ptr2->type, organizationalUnitNameOid);
    ptr2->value.present = AttributeValue_PR_utf8String;
    OCTET_STRING_fromBuf(&ptr2->value.choice.utf8String,
                         info.organizationUnit.data(),
                         info.organizationUnit.size());
    ASN_SEQUENCE_ADD(&ptr->list, ptr2);
    ASN_SEQUENCE_ADD(&rdnSeq.list, ptr);

    ptr = typedMalloc<RelativeDistinguishedName>();
    ptr2 = typedMalloc<AttributeTypeAndValue>();
    OBJECT_IDENTIFIER_set(&ptr2->type, commonNameOid);
    ptr2->value.present = AttributeValue_PR_utf8String;
    OCTET_STRING_fromBuf(&ptr2->value.choice.utf8String, info.commonName.data(),
                         info.commonName.size());
    ASN_SEQUENCE_ADD(&ptr->list, ptr2);
    ASN_SEQUENCE_ADD(&rdnSeq.list, ptr);

    return name;
}

constexpr long ONE_YEAR = 365 * 24 * 60 * 60;

std::string tbsCertToDer(const TBSCertificate& tbsCert) {
    std::string resultStr;
    if (auto ret = der_encode(
            &asn_DEF_TBSCertificate, const_cast<TBSCertificate*>(&tbsCert),
            [](const void* buf, std::size_t size, void* resultStr) {
                *reinterpret_cast<std::string*>(resultStr) +=
                    std::string_view(reinterpret_cast<const char*>(buf), size);
                return 0;
            },
            &resultStr);
        ret.encoded < 0) {
        throw std::runtime_error("DER of TBSCertificate failed at: "s +
                                 ret.failed_type->name);
    }
    return resultStr;
}

}  // namespace

std::string createCertificate(const CertInfo& subject, const CertInfo& issuer,
                              std::uint8_t (&issuerPrivateKey)[32]) {
    Certificate cert{};
    cert.tbsCertificate.version = Version::Version_v3;
    cert.tbsCertificate.serialNumber = 70060;
    OBJECT_IDENTIFIER_set(&cert.tbsCertificate.signature.algorithm,
                          sm2WithSm3Oid);

    cert.tbsCertificate.issuer = createName(issuer);
    cert.tbsCertificate.subject = createName(subject);

    auto now = time(NULL);
    auto oneYearLater = now + ONE_YEAR;
    cert.tbsCertificate.validity.notBefore.present = Time_PR_utcTime;
    asn_time2UT(&cert.tbsCertificate.validity.notBefore.choice.utcTime,
                gmtime(&now), 1);
    cert.tbsCertificate.validity.notAfter.present = Time_PR_utcTime;
    asn_time2UT(&cert.tbsCertificate.validity.notAfter.choice.utcTime,
                gmtime(&oneYearLater), 1);

    auto& pubkey = cert.tbsCertificate.subjectPublicKeyInfo;
    OBJECT_IDENTIFIER_set(&pubkey.algorithm.algorithm, ecPublicKeyOid);
    OBJECT_IDENTIFIER_t* oid = typedMalloc<OBJECT_IDENTIFIER_t>();
    OBJECT_IDENTIFIER_set(oid, sm2EccOid);
    pubkey.algorithm.parameters =
        ANY_new_fromType(&asn_DEF_OBJECT_IDENTIFIER, oid);
    pubkey.subjectPublicKey.buf =
        reinterpret_cast<std::uint8_t*>(std::calloc(1, 65));
    pubkey.subjectPublicKey.size = 65;
    sm2_point_to_uncompressed_octets(&subject.publicKey,
                                     pubkey.subjectPublicKey.buf);

    auto tbsDer = tbsCertToDer(cert.tbsCertificate);
    std::uint8_t digestBuf[32]{};
    sm3_digest(reinterpret_cast<std::uint8_t*>(tbsDer.data()), tbsDer.size(),
               digestBuf);

    OBJECT_IDENTIFIER_set(&cert.signatureAlgorithm.algorithm, sm2WithSm3Oid);

    cert.signatureValue.buf =
        reinterpret_cast<std::uint8_t*>(std::calloc(1, 72));
    SM2_KEY key{};
    std::size_t signatureSize = 72;
    std::ranges::copy(issuerPrivateKey, key.private_key);
    if (sm2_sign(&key, digestBuf, cert.signatureValue.buf, &signatureSize) <
        0) {
        throw std::runtime_error("SM2 sign failed");
    }
    assert(signatureSize <= 72);
    cert.signatureValue.size = signatureSize;

    std::string resultStr;
    if (auto ret = der_encode(
            &asn_DEF_Certificate, &cert,
            [](const void* buf, std::size_t size, void* resultStr) {
                *reinterpret_cast<std::string*>(resultStr) +=
                    std::string_view(reinterpret_cast<const char*>(buf), size);
                return 0;
            },
            &resultStr);
        ret.encoded < 0) {
        std::cout << ret.failed_type->name << std::endl;
    }

    return resultStr;
}

void verifyCertificate(std::string_view cert, SM2_POINT issuerPublicKey) {
    Certificate* certPtr{};
    if (auto ret = ber_decode(nullptr, &asn_DEF_Certificate,
                              reinterpret_cast<void**>(&certPtr), cert.data(),
                              cert.size());
        ret.code != RC_OK) {
        throw std::runtime_error("BER decode failed");
    }
    auto tbsDer = tbsCertToDer(certPtr->tbsCertificate);
    std::uint8_t digest[32];
    sm3_digest(reinterpret_cast<std::uint8_t*>(tbsDer.data()), tbsDer.size(),
               digest);

    SM2_KEY key{.public_key = issuerPublicKey};
    if (sm2_verify(&key, digest, certPtr->signatureValue.buf,
                   certPtr->signatureValue.size) != 1) {
        throw std::runtime_error("Verify failed");
    }
}