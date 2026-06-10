// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>

#include "CoSimTypes.hpp"
#include "Result.hpp"

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

void InitializeOutput();

void LogIoData(std::string_view ioDataStr);
void LogCanMessage(std::string_view canMessageStr);
void LogEthMessage(std::string_view ethMessageStr);
void LogLinMessage(std::string_view linMessageStr);
void LogFrMessage(std::string_view linMessageStr);

[[nodiscard]] int32_t GetChar();

void SetEnvVariable(const std::string& name, const std::string& value);

[[nodiscard]] DsVeosCoSim::Result StartUp();

[[nodiscard]] uint8_t GenerateU8();
[[nodiscard]] uint16_t GenerateU16();
[[nodiscard]] uint32_t GenerateU32();
[[nodiscard]] uint64_t GenerateU64();
[[nodiscard]] int64_t GenerateI64();
[[nodiscard]] size_t GenerateSizeT();

void FillWithRandomData(uint8_t* data, size_t length);

template <typename T>
[[nodiscard]] T GenerateRandom(T min, T max) {
    uint32_t diff = static_cast<uint32_t>(max) + 1 - static_cast<uint32_t>(min);
    return static_cast<T>(static_cast<uint32_t>(min) + (GenerateU32() % diff));
}

[[nodiscard]] std::string GenerateString(std::string_view prefix);
[[nodiscard]] DsVeosCoSim::SimulationTime GenerateSimulationTime();
[[nodiscard]] DsVeosCoSim::BusMessageId GenerateBusMessageId(uint32_t min, uint32_t max);
[[nodiscard]] DsVeosCoSim::BusControllerId GenerateBusControllerId();
[[nodiscard]] DsVeosCoSim::IoSignalId GenerateIoSignalId();
[[nodiscard]] std::vector<uint8_t> GenerateBytes(size_t length);

[[nodiscard]] DsVeosCoSim::IoSignalContainer CreateSignal();
[[nodiscard]] DsVeosCoSim::IoSignalContainer CreateSignal(DsVeosCoSim::DataType dataType);
[[nodiscard]] DsVeosCoSim::IoSignalContainer CreateSignal(DsVeosCoSim::DataType dataType, DsVeosCoSim::SizeKind sizeKind);

[[nodiscard]] std::vector<uint8_t> GenerateIoData(const DsVeosCoSim::IoSignalContainer& signal);
[[nodiscard]] std::vector<uint8_t> CreateZeroedIoData(const DsVeosCoSim::IoSignalContainer& signal);

void FillWithRandom(DsVeosCoSim::CanControllerContainer& controller);
void FillWithRandom(DsVeosCoSim::EthControllerContainer& controller);
void FillWithRandom(DsVeosCoSim::LinControllerContainer& controller);
void FillWithRandom(DsVeosCoSim::FrControllerContainer& controller);

void FillWithRandom(DsVeosCoSim::CanMessageContainer& message, DsVeosCoSim::BusControllerId controllerId);
void FillWithRandom(DsVeosCoSim::EthMessageContainer& message, DsVeosCoSim::BusControllerId controllerId);
void FillWithRandom(DsVeosCoSim::LinMessageContainer& message, DsVeosCoSim::BusControllerId controllerId);
void FillWithRandom(DsVeosCoSim::FrMessageContainer& message, DsVeosCoSim::BusControllerId controllerId);

[[nodiscard]] std::vector<DsVeosCoSim::IoSignalContainer> CreateSignals(size_t count);
[[nodiscard]] std::vector<DsVeosCoSim::CanControllerContainer> CreateCanControllers(size_t count);
[[nodiscard]] std::vector<DsVeosCoSim::EthControllerContainer> CreateEthControllers(size_t count);
[[nodiscard]] std::vector<DsVeosCoSim::LinControllerContainer> CreateLinControllers(size_t count);
[[nodiscard]] std::vector<DsVeosCoSim::FrControllerContainer> CreateFrControllers(size_t count);
