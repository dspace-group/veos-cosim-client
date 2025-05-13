// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <string>

[[maybe_unused]] constexpr uint32_t BufferSize = 24U;
[[maybe_unused]] constexpr uint16_t UdpPort = 27100;
[[maybe_unused]] constexpr uint16_t TcpPort = 27101;
[[maybe_unused]] constexpr uint16_t CommunicationPort = 27102;
[[maybe_unused]] constexpr uint16_t CoSimPort = 27103;
const std::string UdsName = "Uds4711";
const std::string PipeName = "Pipe4711";
const std::string BeginEventName = "BeginEvent4711";
const std::string EndEventName = "EndEvent4711";
const std::string ShmName = "Shm4711";
const std::string LocalName = "Local4711";
const std::string CoSimServerName = "TestServer";
