#include <variant>

#include "cli.hpp"
#include "encrypt.hpp"
#include "keygen.hpp"

int main(int argc, char** argv) try {
    auto opt = parseCommandLine(argc, argv);
    std::visit(
        [](auto opt) {
            using namespace encrypt;
            using namespace keygen;
            entry(std::move(opt));
        },
        std::move(opt));
} catch (std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    std::exit(1);
}
