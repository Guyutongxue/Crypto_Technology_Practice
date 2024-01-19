
#include <cstdint>
#include <format>
#include <iostream>

#include "handler.hpp"

constexpr const std::uint16_t DEFAULT_PORT = 8080;
constexpr const std::string DEFAULT_HOSTNAME = "0.0.0.0";

int main() try {
    initDB();
    Server server
#ifdef USE_SSL
        ("cert.pem", "key.pem")
#endif
            ;
    createServer(server);

    std::cout << std::format("Listening on {}:{}\n", DEFAULT_HOSTNAME,
                             DEFAULT_PORT);
    if (!server.listen(DEFAULT_HOSTNAME, DEFAULT_PORT)) {
        throw std::runtime_error("failed to listen");
    }
} catch (std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
}