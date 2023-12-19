#pragma once

#include <httplib.h>

#include "base64.hpp"
#include "db.hpp"
#include "log.hpp"

#ifdef USE_SSL
using Server = httplib::SSLServer;
#else
using Server = httplib::Server;
#endif

inline void createServer(Server& server) {
    using namespace httplib;
    server.Put("/users/:id", [](const Request& req, Response& res) {
        auto userId = req.path_params.at("id");
        try {
            writeLog("register user {}", userId);
            registerUser(userId);
            res.set_content("ok", "text/plain");
            res.status = 201;
        } catch (std::exception& e) {
            res.set_content(e.what(), "text/plain");
            writeLog("ERROR: {}", e.what());
            res.status = 400;
        }
    });
    server.Get("/users/:id/publicKey", [](const Request& req, Response& res) {
        auto userId = req.path_params.at("id");
        try {
            writeLog("get public key of user {}", userId);
            auto publicKey = getPublicKey(userId);
            res.set_content(publicKey, "text/plain");
            res.status = 200;
        } catch (std::exception& e) {
            res.set_content(e.what(), "text/plain");
            writeLog("ERROR: {}", e.what());
            res.status = 400;
        }
    });
    server.Post("/decrypt", [](const Request& req, Response& res) {
        auto userId = req.get_header_value("X-User");
        auto input = req.body;
        try {
            writeLog("decrypt request from user {}", userId);
            auto privateKey = getPrivateKey(userId);
            auto rawInput = base64Decode(input);
            auto decrypted = sm2::decrypt(rawInput, privateKey);
            auto base64Output = base64Encode(decrypted);
            res.set_content(base64Output, "text/plain");
            res.status = 200;
        } catch (std::exception& e) {
            res.set_content(e.what(), "text/plain");
            writeLog("ERROR: {}", e.what());
            res.status = 400;
        }
    });
}