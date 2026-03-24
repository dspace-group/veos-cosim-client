// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <string_view>

#include "Logger.hpp"  // IWYU pragma: keep

namespace DsVeosCoSim {

enum class Result {
    Ok,
    Error,
    Empty,
    Full,
    InvalidArgument,
    NotConnected,
    Timeout
};

[[nodiscard]] constexpr std::string_view format_as(Result result) {
    switch (result) {
        case Result::Ok:
            return "Ok";
        case Result::Error:
            return "Error";
        case Result::Empty:
            return "Empty";
        case Result::Full:
            return "Full";
        case Result::InvalidArgument:
            return "InvalidArgument";
        case Result::NotConnected:
            return "NotConnected";
        case Result::Timeout:
            return "Timeout";
    }

    return "<Invalid Result>";
}

[[nodiscard]] constexpr bool IsOk(Result result) noexcept {
    return result == Result::Ok;
}

[[nodiscard]] constexpr bool IsError(Result result) noexcept {
    return result == Result::Error;
}

[[nodiscard]] constexpr bool IsTimeout(Result result) noexcept {
    return result == Result::Timeout;
}

[[nodiscard]] constexpr bool IsNotConnected(Result result) noexcept {
    return result == Result::NotConnected;
}

[[nodiscard]] constexpr bool IsFull(Result result) noexcept {
    return result == Result::Full;
}

constexpr Result CreateOk() noexcept {
    return Result::Ok;
}

constexpr Result CreateError() noexcept {
    return Result::Error;
}

constexpr Result CreateTimeout() noexcept {
    return Result::Timeout;
}

constexpr Result CreateNotConnected() noexcept {
    return Result::NotConnected;
}

constexpr Result CreateInvalidArgument() noexcept {
    return Result::InvalidArgument;
}

constexpr Result CreateEmpty() noexcept {
    return Result::Empty;
}

constexpr Result CreateFull() noexcept {
    return Result::Full;
}

#define CheckResult(expr)       \
    do {                        \
        auto _result_ = (expr); \
        if (!IsOk(_result_)) {  \
            return _result_;    \
        }                       \
    } while (false)

#define CheckResultWithMessage(expr, message) \
    do {                                      \
        auto _result_ = (expr);               \
        if (!IsOk(_result_)) {                \
            LogTrace(message);                \
            return _result_;                  \
        }                                     \
    } while (false)

}  // namespace DsVeosCoSim
