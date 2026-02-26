// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include "Logger.hpp"  // IWYU pragma: keep

namespace DsVeosCoSim {

enum class ResultKind {
    Ok,
    Error,
    Timeout,
    NotConnected,
    Empty,
    Full,
    InvalidArgument
};

[[nodiscard]] constexpr const char* format_as(ResultKind ResultKind) {
    switch (ResultKind) {
        case ResultKind::Ok:
            return "Ok";
        case ResultKind::Error:
            return "Error";
        case ResultKind::Timeout:
            return "Timeout";
        case ResultKind::NotConnected:
            return "NotConnected";
        case ResultKind::Empty:
            return "Empty";
        case ResultKind::Full:
            return "Full";
        case ResultKind::InvalidArgument:
            return "InvalidArgument";
    }
    return "<Invalid ResultKind>";
}

struct Result final {
    ResultKind kind{};
};

[[nodiscard]] constexpr const char* format_as(const Result& result) {
    return format_as(result.kind);
}

[[nodiscard]] constexpr bool IsOk(const Result& result) noexcept {
    return result.kind == ResultKind::Ok;
}

[[nodiscard]] constexpr bool IsError(const Result& result) noexcept {
    return result.kind == ResultKind::Error;
}

[[nodiscard]] constexpr bool IsTimeout(const Result& result) noexcept {
    return result.kind == ResultKind::Timeout;
}

[[nodiscard]] constexpr bool IsNotConnected(const Result& result) noexcept {
    return result.kind == ResultKind::NotConnected;
}

constexpr Result CreateOk() noexcept {
    return {ResultKind::Ok};
}

constexpr Result CreateError() noexcept {
    return {ResultKind::Error};
}

constexpr Result CreateTimeout() noexcept {
    return {ResultKind::Timeout};
}

constexpr Result CreateNotConnected() noexcept {
    return {ResultKind::NotConnected};
}

constexpr Result CreateInvalidArgument() noexcept {
    return {ResultKind::InvalidArgument};
}

constexpr Result CreateEmpty() noexcept {
    return {ResultKind::Empty};
}

constexpr Result CreateFull() noexcept {
    return {ResultKind::Full};
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
