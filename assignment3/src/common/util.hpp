#pragma once

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>

#include <cstdint>
#include <format>
#include <string>
#include <string_view>

template <typename F>
class Defer {
public:
    Defer(F f) : f(f) {}
    ~Defer() {
        f();
    }

private:
    F f;
};

struct DeferHelper {};

template <typename F>
Defer<F> operator*(DeferHelper, F f) {
    return Defer<F>(f);
}

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define defer auto CONCAT(defer_, __LINE__) = DeferHelper{}* [&]()

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

#define SSL_CHECK(x)                                                   \
    do {                                                               \
        if (!(x)) {                                                    \
            ERR_print_errors_fp(stderr);                               \
            throw std::runtime_error(__FILE__                          \
                                     ":" STRINGIFY(__LINE__) ": " #x); \
        }                                                              \
    } while (0)

using UString = std::basic_string<std::uint8_t>;
using UStringView = std::basic_string_view<std::uint8_t>;

template <std::forward_iterator It>
    requires std::same_as<std::iter_value_t<It>, std::uint8_t>
inline std::string hex(It data, std::size_t len) {
    auto ret = std::format("({}) ", len);
    ret.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        ret += std::format("{:02x}", *(data++));
    }
    return ret;
}

template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, std::uint8_t>
inline std::string hex(const R& r) {
    return hex(std::ranges::begin(r), std::ranges::size(r));
}
