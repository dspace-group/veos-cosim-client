// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <stdexcept>  // IWYU pragma: keep
#include <string>
#include <string_view>

#include "Channel.h"
#include "Socket.h"

constexpr uint32_t Infinite = UINT32_MAX;  // NOLINT

#define MUST_BE_TRUE(actual)                             \
    do {                                                 \
        if (!(actual)) {                                 \
            throw std::runtime_error("Invalid result."); \
        }                                                \
    } while (0)

constexpr uint32_t DefaultTimeout = 1000;  // NOLINT

[[nodiscard]] bool StartUp();

[[nodiscard]] DsVeosCoSim::Socket ConnectSocket(std::string_view ipAddress, uint16_t remotePort);
[[nodiscard]] DsVeosCoSim::Socket ConnectSocket(const std::string& name);
[[nodiscard]] DsVeosCoSim::Socket Accept(const DsVeosCoSim::Socket& serverSocket);

[[nodiscard]] std::unique_ptr<DsVeosCoSim::Channel> ConnectToTcpChannel(std::string_view ipAddress,
                                                                        uint16_t remotePort);
[[nodiscard]] std::unique_ptr<DsVeosCoSim::Channel> ConnectToUdsChannel(const std::string& name);

#ifdef _WIN32

[[nodiscard]] std::unique_ptr<DsVeosCoSim::Channel> ConnectToLocalChannel(const std::string& name);

#endif

[[nodiscard]] std::unique_ptr<DsVeosCoSim::Channel> Accept(DsVeosCoSim::ChannelServer& server);

[[nodiscard]] std::string_view GetLoopBackAddress(DsVeosCoSim::AddressFamily addressFamily);

[[nodiscard]] bool SendComplete(const DsVeosCoSim::Socket& socket, const void* buffer, size_t length);

[[nodiscard]] bool ReceiveComplete(const DsVeosCoSim::Socket& socket, void* buffer, size_t length);
