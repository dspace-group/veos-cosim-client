// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "DsVeosCoSim/CoSimTypes.h"

[[nodiscard]] int32_t Random(int32_t min, int32_t max);

void FillWithRandom(uint8_t* data, size_t length);

template <typename T>
[[nodiscard]] T GenerateRandom(T min, T max) {
    return static_cast<T>(Random(static_cast<int32_t>(min), static_cast<int32_t>(max)));
}

[[nodiscard]] uint8_t GenerateU8();
[[nodiscard]] uint16_t GenerateU16();
[[nodiscard]] uint32_t GenerateU32();
[[nodiscard]] uint32_t GenerateU32(uint32_t min, uint32_t max);
[[nodiscard]] uint64_t GenerateU64();
[[nodiscard]] int64_t GenerateI64();
[[nodiscard]] std::string GenerateString(std::string_view prefix);
[[nodiscard]] DsVeosCoSim::SimulationTime GenerateSimulationTime();
[[nodiscard]] DsVeosCoSim::BusMessageId GenerateBusMessageId(uint32_t min, uint32_t max);
[[nodiscard]] std::vector<uint8_t> GenerateBytes(size_t length);

[[nodiscard]] DsVeosCoSim::IoSignalContainer CreateSignal();
[[nodiscard]] DsVeosCoSim::IoSignalContainer CreateSignal(DsVeosCoSim::DataType dataType);
[[nodiscard]] DsVeosCoSim::IoSignalContainer CreateSignal(DsVeosCoSim::DataType dataType,
                                                          DsVeosCoSim::SizeKind sizeKind);

[[nodiscard]] std::vector<uint8_t> GenerateIoData(const DsVeosCoSim::IoSignalContainer& signal);
[[nodiscard]] std::vector<uint8_t> CreateZeroedIoData(const DsVeosCoSim::IoSignalContainer& signal);

void FillWithRandom(DsVeosCoSim::CanControllerContainer& controller);
void FillWithRandom(DsVeosCoSim::EthControllerContainer& controller);
void FillWithRandom(DsVeosCoSim::LinControllerContainer& controller);

void FillWithRandom(DsVeosCoSim::CanMessageContainer& message, DsVeosCoSim::BusControllerId controllerId);
void FillWithRandom(DsVeosCoSim::EthMessageContainer& message, DsVeosCoSim::BusControllerId controllerId);
void FillWithRandom(DsVeosCoSim::LinMessageContainer& message, DsVeosCoSim::BusControllerId controllerId);

[[nodiscard]] std::vector<DsVeosCoSim::IoSignalContainer> CreateSignals(size_t count);

[[nodiscard]] std::vector<DsVeosCoSim::CanControllerContainer> CreateCanControllers(size_t count);
[[nodiscard]] std::vector<DsVeosCoSim::EthControllerContainer> CreateEthControllers(size_t count);
[[nodiscard]] std::vector<DsVeosCoSim::LinControllerContainer> CreateLinControllers(size_t count);
