#include <algorithm>

#include "cli.hpp"
#include "crypt.hpp"

int main(int argc, char** argv) try {
    auto opt = parseCommandLine(argc, argv);
    std::visit([](auto opt) { action::run(std::move(opt)); }, std::move(opt));
} catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
}
