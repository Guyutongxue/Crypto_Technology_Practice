#ifdef _WIN32
#warning This program has not been tested under Windows. You may occur compile errors and/or runtime errors.
#endif

#include <format>
#include <iostream>
#include <string>

#include "cli.hpp"
#include "encrypt.hpp"
#include "keygen.hpp"

int main(int argc, char** argv) try {
    // auto& message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.
    // Sed non risus. Suspendisse lectus tortor, dignissim sit amet, adipiscing
    // nec, ultricies sed, dolor."; SM2_KEY sm2key; sm2_key_generate(&sm2key);
    // auto pubkey = base64Encode(podToSv(sm2key.public_key));
    // auto privkey = base64Encode(podToSv(sm2key.private_key));
    // std::ranges::fill(sm2key.private_key, 0);
    // SM2_KEY sm2key2;
    // svToPod(base64Decode(privkey), sm2key2.private_key);
    // printRange(sm2key2.private_key);
    // auto cipher = detail::sm2Encrypt(sm2key, message);
    // auto plain = detail::sm2Decrypt(sm2key2, cipher);
    // std::cout << plain << std::endl;
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
