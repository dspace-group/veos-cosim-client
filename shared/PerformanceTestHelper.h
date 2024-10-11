// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <string>

constexpr uint32_t BufferSize = 24U;                  // NOLINT
constexpr uint16_t UdpPort = 27100;                   // NOLINT
constexpr uint16_t TcpPort = 27101;                   // NOLINT
constexpr uint16_t CommunicationPort = 27102;         // NOLINT
constexpr uint16_t CoSimPort = 27103;                 // NOLINT
const std::string UdsName = "Uds4711";                // NOLINT
const std::string PipeName = "Pipe4711";              // NOLINT
const std::string BeginEventName = "BeginEvent4711";  // NOLINT
const std::string EndEventName = "EndEvent4711";      // NOLINT
const std::string ShmName = "Shm4711";                // NOLINT
const std::string LocalName = "Local4711";            // NOLINT
const std::string CoSimServerName = "TestServer";     // NOLINT
