
#include <cstdint>
#include <iostream>
#include <format>

#include "handler.hpp"
#include "httplib.h"

constexpr const std::uint16_t DEFAULT_PORT = 8080;
constexpr const std::string DEFAULT_HOSTNAME = "0.0.0.0";

int main() {
    initDB();
    auto server = createServer();
    std::cout << std::format("Listening on {}:{}\n", DEFAULT_HOSTNAME, DEFAULT_PORT);
    server.listen(DEFAULT_HOSTNAME, DEFAULT_PORT);
}