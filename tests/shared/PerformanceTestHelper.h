// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>

[[maybe_unused]] constexpr uint32_t BufferSize = 24U;
[[maybe_unused]] constexpr uint16_t UdpPort = 27100;
[[maybe_unused]] constexpr uint16_t TcpPort = 27101;
[[maybe_unused]] constexpr uint16_t CommunicationPort = 27102;
[[maybe_unused]] constexpr uint16_t CoSimPort = 27103;
[[maybe_unused]] const auto UdsName = "Uds4711";
[[maybe_unused]] const auto PipeName = "Pipe4711";
[[maybe_unused]] const auto BeginEventName = "BeginEvent4711";
[[maybe_unused]] const auto EndEventName = "EndEvent4711";
[[maybe_unused]] const auto ShmName = "Shm4711";
[[maybe_unused]] const auto LocalName = "Local4711";
[[maybe_unused]] const auto CoSimServerName = "TestServer";
