#include <algorithm>

#include "cli.hpp"
#include "crypt.hpp"

int main(int argc, char** argv) {
    auto opt = parseCommandLine(argc, argv);
    std::visit([](auto opt) { action::run(std::move(opt)); }, std::move(opt));
}
