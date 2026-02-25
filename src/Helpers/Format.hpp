// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace DsVeosCoSim {

inline std::string format_as(const std::string& s) {
    return s;
}

inline std::string format_as(const char* s) {
    return (s != nullptr) ? std::string(s) : std::string();
}

inline std::string format_as(std::string_view s) {
    return std::string(s);
}

template <typename T>
std::enable_if_t<std::is_integral_v<T>, std::string> format_as(T value) {
    return std::to_string(value);
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, std::string> format_as(T value) {
    std::string str = std::to_string(value);

    // Remove trailing zeros
    while (!str.empty() && (str.back() == '0')) {
        str.pop_back();
    }

    // Remove trailing point
    if (!str.empty() && (str.back() == '.')) {
        str.pop_back();
    }

    return str;
}

namespace detail {

inline void FormatImpl(std::string& out, const char* fmt) {
    // fmt is a null-terminated string, that means, if *fmt is not \0, fmt[1] is accessible
    while (*fmt != 0) {
        char currentChar = fmt[0];
        char nextChar = fmt[1];

        if ((currentChar == '{') && (nextChar == '{')) {
            out.push_back('{');
            fmt += 2;
        } else if ((currentChar == '}') && (nextChar == '}')) {
            out.push_back('}');
            fmt += 2;
        } else {
            out.push_back(currentChar);
            fmt++;
        }
    }
}

template <typename T, typename... Args>
void FormatImpl(std::string& out, const char* fmt, T&& value, Args&&... args) {
    // fmt is a null-terminated string, that means, if *fmt is not \0, fmt[1] is accessible
    while (*fmt != 0) {
        char currentChar = fmt[0];
        char nextChar = fmt[1];

        if (currentChar == '{') {
            if (nextChar == '{') {
                out.push_back('{');
                fmt += 2;
                continue;
            }

            if (nextChar == '}') {
                out.append(format_as(std::forward<T>(value)));
                FormatImpl(out, fmt + 2, std::forward<Args>(args)...);
                return;
            }
        } else if ((currentChar == '}') && (nextChar == '}')) {
            out.push_back('}');
            fmt += 2;
            continue;
        }

        out.push_back(currentChar);
        fmt++;
    }

    // This is a "high-level" assert. Hopefully, this will never be reached
    throw std::runtime_error("Too many arguments");
}

}  // namespace detail

template <typename... Args>
std::string Format(const std::string& fmt, Args&&... args) {
    std::string out;
    out.reserve(fmt.size() + (sizeof...(Args) * 8));
    detail::FormatImpl(out, fmt.c_str(), std::forward<Args>(args)...);
    return out;
}

}  // namespace DsVeosCoSim
