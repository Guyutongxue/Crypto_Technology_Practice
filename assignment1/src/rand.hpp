#pragma once

#include <algorithm>
#include <concepts>
#include <random>

template <std::integral IntT, std::uniform_random_bit_generator GenT>
inline IntT randomInt(GenT& generator) {
    // [rand.req.genl] prohibit generating random char
    using TargetIntT = std::conditional_t<sizeof(IntT) < sizeof(short), short, IntT>;
    static std::uniform_int_distribution<TargetIntT> d;
    return d(generator);
}

template <std::integral IntT = std::size_t>
inline IntT randomInt() {
    static std::random_device rd;
    return randomInt<IntT>(rd);
}

template <std::integral IntT, std::size_t N>
inline std::array<IntT, N> randomIntArray(std::size_t seed = randomInt<std::size_t>()) {
    std::default_random_engine generator(seed);
    std::array<IntT, N> arr;
    std::ranges::generate(arr, [&] { return randomInt<IntT>(generator); });
    return arr;
}