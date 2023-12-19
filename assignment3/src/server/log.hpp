#pragma once

#include <format>
#include <fstream>
#include <iostream>

template <typename... Args>
inline void writeLog(std::format_string<Args...> fmt, Args&&... args) {
    auto msg = std::format(fmt, std::forward<Args>(args)...);
    std::clog << msg << '\n';
    std::ofstream ofs("server.log", std::ios::app);
    ofs << msg << '\n';
}
