#pragma once

#include <SQLiteCpp/SQLiteCpp.h>

#include <string>

#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Statement.h"
#include "sm2.hpp"

constexpr std::string DB_PATH = "test.db";

inline void initDB() {
    SQLite::Database db(DB_PATH, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    db.exec(R"(
CREATE TABLE IF NOT EXISTS users (
    name TEXT PRIMARY KEY,
    pubKey TEXT,
    privKey TEXT
))");
}

inline std::string getPublicKey(const std::string& user) {
    SQLite::Database db(DB_PATH, SQLite::OPEN_READONLY);
    SQLite::Statement query(db, "SELECT pubKey FROM users WHERE name = ?");
    query.bind(1, user);
    if (query.executeStep()) {
        return query.getColumn(0);
    } else {
        throw std::runtime_error("user not found");
    }
}

inline std::string getPrivateKey(const std::string& user) {
    SQLite::Database db(DB_PATH, SQLite::OPEN_READONLY);
    SQLite::Statement query(db, "SELECT privKey FROM users WHERE name = ?");
    query.bind(1, user);
    if (query.executeStep()) {
        return query.getColumn(0);
    } else {
        throw std::runtime_error("user not found");
    }
}

inline void registerUser(const std::string& user) {
    SQLite::Database db(DB_PATH, SQLite::OPEN_READWRITE);
    auto [pubKey, privKey] = sm2::generateKey();

    SQLite::Statement query(db, "INSERT INTO users VALUES (?, ?, ?)");
    query.bind(1, user);
    query.bind(2, pubKey);
    query.bind(3, privKey);
    query.exec();
}