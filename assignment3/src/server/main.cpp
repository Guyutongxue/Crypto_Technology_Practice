
#include <cstdint>
#include <format>
#include <iostream>

#include "handler.hpp"
#include "httplib.h"

constexpr const std::uint16_t DEFAULT_PORT = 8080;
constexpr const std::string DEFAULT_HOSTNAME = "0.0.0.0";

int main() {
    initDB();
    Server server
#ifdef USE_SSL
        ("cert.pem", "key.pem")
#endif
            ;
    createServer(server);

    std::cout << std::format("Listening on {}:{}\n", DEFAULT_HOSTNAME,
                             DEFAULT_PORT);
    server.listen(DEFAULT_HOSTNAME, DEFAULT_PORT);
}