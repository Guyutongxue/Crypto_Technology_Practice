#pragma once

#include <format>
#include <fstream>
#include <iostream>
#include <chrono>

template <typename... Args>
inline void writeLog(std::format_string<Args...> fmt, Args&&... args) {
    auto msg = std::format("[{}] ", std::chrono::system_clock::now()) + std::format(fmt, std::forward<Args>(args)...);
    std::clog << msg << '\n';
    std::ofstream ofs("server.log", std::ios::app);
    ofs << msg << '\n';
}
