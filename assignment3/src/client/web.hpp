#pragma once

#include <httplib.h>

#include <stdexcept>
#include <string>

#include "base64.hpp"
#include "cli.hpp"

#ifdef USE_SSL
const std::string PROTOCOL = "https:";
#else
const std::string PROTOCOL = "http:";
#endif

inline std::string getPublicKeyFromServer(const Config& config) {
    httplib::Client cli(
        std::format("{}//{}:{}", PROTOCOL, config.hostname, config.port));
    auto res = cli.Get(std::format("/users/{}/publicKey", config.user));
    if (res.error() != httplib::Error::Success) {
        throw std::runtime_error("cannot get public key from server: " + res->body);
    }
    return res->body;
}

inline UString decryptFromServer(const Config& config, UStringView input) {
    httplib::Client cli(
        std::format("{}//{}:{}", PROTOCOL, config.hostname, config.port));

    httplib::Headers headers{
        {"X-User", config.user}
    };
    auto body = base64Encode(input);
    auto res = cli.Post("/decrypt", headers, body, "text/plain");
    if (res.error() != httplib::Error::Success) {
        throw std::runtime_error("cannot decrypt from server: " + res->body);
    }
    return base64Decode(res->body);
}

namespace action {

inline void run(RegisterOption opt) {
    httplib::Client cli(
        std::format("{}//{}:{}", PROTOCOL, opt.config.hostname, opt.config.port));
    auto res =cli.Put(std::format("/users/{}", opt.config.user));
    if (res.error() != httplib::Error::Success) {
        throw std::runtime_error("cannot register user: " + res->body);
    }
}

}