#pragma once

#include <httplib.h>

#include "base64.hpp"
#include "db.hpp"

#ifdef USE_SSL
using Server = httplib::SSLServer;
#else
using Server = httplib::Server;
#endif

inline Server createServer() {
    Server server
#ifdef USE_SSL
        ("guyutongxue.crt", "guyutongxue.key")
#endif
            ;
    using namespace httplib;
    server.Put("/users/:id", [](const Request& req, Response& res) {
        auto userId = req.path_params.at("id");
        try {
            registerUser(userId);
            res.set_content("ok", "text/plain");
            res.status = 201;
        } catch (std::exception& e) {
            res.set_content(e.what(), "text/plain");
            res.status = 400;
        }
    });
    server.Get("/users/:id/publicKey", [](const Request& req, Response& res) {
        auto userId = req.path_params.at("id");
        try {
            auto publicKey = getPublicKey(userId);
            res.set_content(publicKey, "text/plain");
            res.status = 200;
        } catch (std::exception& e) {
            res.set_content(e.what(), "text/plain");
            res.status = 400;
        }
    });
    server.Post("/decrypt", [](const Request& req, Response& res) {
        auto userId = req.get_header_value("X-User");
        auto input = req.body;
        try {
            auto privateKey = getPrivateKey(userId);
            auto rawInput = base64Decode(input);
            auto decrypted = sm2::decrypt(rawInput, privateKey);
            auto base64Output = base64Encode(decrypted);
            res.set_content(base64Output, "text/plain");
            res.status = 200;
        } catch (std::exception& e) {
            res.set_content(e.what(), "text/plain");
            res.status = 400;
        }
    });
}