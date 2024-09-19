// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <string_view>

constexpr uint32_t BufferSize = 24U;                           // NOLINT
constexpr uint16_t UdpPort = 27100;                            // NOLINT
constexpr uint16_t TcpPort = 27101;                            // NOLINT
constexpr uint16_t CommunicationPort = 27102;                  // NOLINT
constexpr uint16_t CoSimPort = 27103;                          // NOLINT
constexpr uint16_t AsioAsyncPort = 27111;                      // NOLINT
constexpr uint16_t AsioBlockingPort = 27112;                   // NOLINT
constexpr std::string_view UdsName = "Uds4711";                // NOLINT
constexpr std::string_view PipeName = "Pipe4711";              // NOLINT
constexpr std::string_view BeginEventName = "BeginEvent4711";  // NOLINT
constexpr std::string_view EndEventName = "EndEvent4711";      // NOLINT
constexpr std::string_view ShmName = "Shm4711";                // NOLINT
constexpr std::string_view LocalName = "Local4711";            // NOLINT
constexpr std::string_view CoSimServerName = "TestServer";     // NOLINT
