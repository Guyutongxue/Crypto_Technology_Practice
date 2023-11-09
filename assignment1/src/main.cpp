#include <format>
#include <iostream>
#include <string>

#include "cli.hpp"
#include "encrypt.hpp"
#include "keygen.hpp"

int main(int argc, char** argv) try {
    auto opt = parseCommandLine(argc, argv);
    std::visit(
        []<typename T>(T&& opt) {
            if constexpr (std::is_same_v<std::decay_t<T>, KeygenOption>) {
                keygen(std::move(opt));
            } else {
                encrypt(std::move(opt));
            }
        },
        std::move(opt));
} catch (...) {
    throw;
}
