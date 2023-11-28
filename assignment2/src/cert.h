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

#pragma once

#include <gmssl/sm2.h>

#include <cstdint>
#include <string>

struct CertInfo {
    std::string country;
    std::string organization;
    std::string organizationUnit;
    std::string commonName;
    SM2_POINT publicKey;
};

std::string createCertificate(const CertInfo& subject, const CertInfo& issuer,
                              std::uint8_t (&issuerPrivateKey)[32]);

void verifyCertificate(std::string_view cert, SM2_POINT issuerPublicKey);
