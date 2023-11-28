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

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <type_traits>

#include "cert.h"

template <typename T>
    requires std::is_trivially_copyable_v<T>
void writeFile(const std::filesystem::path& p, const T& content) {
    std::ofstream ofs(p, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(&content), sizeof(content));
}
void writeFile(const std::filesystem::path& p, std::string_view content) {
    std::ofstream ofs(p, std::ios::binary);
    ofs.write(content.data(), content.size());
}

SM2_KEY keygen(const std::filesystem::path& p) {
    SM2_KEY sm2key;
    if (sm2_key_generate(&sm2key) < 0) {
        throw std::runtime_error("sm2_key_generate failed");
    }
    std::filesystem::create_directories(p);
    writeFile(p / "private.key", sm2key.private_key);
    writeFile(p / "public.key", sm2key.public_key);
    return sm2key;
}

int main() {
    auto [rootPub, rootPriv] = keygen("root");
    auto issuer = CertInfo{
        .country = "CN",
        .organization = "中国科学院大学",
        .organizationUnit = "网安学院",
        .commonName = "谷雨",
        .publicKey = rootPub,
    };

    auto [subPub, subPriv] = keygen("sub");
    auto subSubject = CertInfo{
        .country = "CN",
        .organization = "中国科学院大学",
        .organizationUnit = "网安学院",
        .commonName = "谷雨的证书1",
        .publicKey = subPub,
    };

    auto rootCert = createCertificate(issuer, issuer, rootPriv);
    auto subCert = createCertificate(subSubject, issuer, rootPriv);

    writeFile("root.crt", rootCert);
    writeFile("sub.crt", subCert);

    // asn1_delete_structure(&certDef);

    verifyCertificate(rootCert, rootPub);
    verifyCertificate(subCert, rootPub);
}