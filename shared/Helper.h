// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <stdexcept>  // IWYU pragma: keep
#include <string>
#include <string_view>

#include "Communication/SocketChannel.h"
#include "OsAbstraction/Socket.h"

#ifdef _WIN32
#include "Communication/LocalChannel.h"
#endif

constexpr uint32_t Infinite = UINT32_MAX;  // NOLINT

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

#define MUST_BE_TRUE(actual)                             \
    do {                                                 \
        if (!(actual)) {                                 \
            throw std::runtime_error("Invalid result."); \
        }                                                \
    } while (0)

[[nodiscard]] int32_t GetChar();

constexpr uint32_t DefaultTimeout = 1000;  // NOLINT

[[nodiscard]] bool StartUp();

[[nodiscard]] DsVeosCoSim::Socket ConnectSocket(std::string_view ipAddress, uint16_t remotePort);
[[nodiscard]] DsVeosCoSim::Socket ConnectSocket(const std::string& name);
[[nodiscard]] DsVeosCoSim::Socket Accept(const DsVeosCoSim::Socket& serverSocket);

[[nodiscard]] DsVeosCoSim::SocketChannel ConnectToTcpChannel(std::string_view ipAddress, uint16_t remotePort);
[[nodiscard]] DsVeosCoSim::SocketChannel Accept(const DsVeosCoSim::TcpChannelServer& server);

[[nodiscard]] DsVeosCoSim::SocketChannel ConnectToUdsChannel(const std::string& name);
[[nodiscard]] DsVeosCoSim::SocketChannel Accept(const DsVeosCoSim::UdsChannelServer& server);

#ifdef _WIN32

[[nodiscard]] DsVeosCoSim::LocalChannel ConnectToLocalChannel(const std::string& name);
[[nodiscard]] DsVeosCoSim::LocalChannel Accept(DsVeosCoSim::LocalChannelServer& server);

#endif

[[nodiscard]] std::string_view GetLoopBackAddress(DsVeosCoSim::AddressFamily addressFamily);

[[nodiscard]] bool SendComplete(const DsVeosCoSim::Socket& socket, const void* buffer, size_t length);

[[nodiscard]] bool ReceiveComplete(const DsVeosCoSim::Socket& socket, void* buffer, size_t length);
