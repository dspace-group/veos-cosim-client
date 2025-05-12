// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <stdexcept>  // IWYU pragma: keep
#include <string_view>

#include "Socket.h"

[[maybe_unused]] constexpr uint32_t Infinite = UINT32_MAX;

#define MUST_BE_TRUE(actual)                             \
    do {                                                 \
        if (!(actual)) {                                 \
            throw std::runtime_error("Invalid result."); \
        }                                                \
    } while (0)

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

[[maybe_unused]] constexpr uint32_t DefaultTimeout = 1000;

[[nodiscard]] int32_t GetChar();

[[nodiscard]] bool StartUp();

[[nodiscard]] std::string_view GetLoopBackAddress(DsVeosCoSim::AddressFamily addressFamily);

[[nodiscard]] bool SendComplete(const DsVeosCoSim::Socket& socket, const void* buffer, size_t length);

[[nodiscard]] bool ReceiveComplete(const DsVeosCoSim::Socket& socket, void* buffer, size_t length);
