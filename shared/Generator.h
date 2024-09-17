// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <string>

#include "BusBuffer.h"
#include "CoSimTypes.h"

[[nodiscard]] int32_t Random(int32_t min, int32_t max);

void FillWithRandom(uint8_t* data, size_t length);

template <typename T>
[[nodiscard]] T GenerateRandom(T min, T max) {
    return static_cast<T>(Random(static_cast<int32_t>(min), static_cast<int32_t>(max)));
}

[[nodiscard]] uint8_t GenerateU8();
[[nodiscard]] uint16_t GenerateU16();
[[nodiscard]] uint32_t GenerateU32();
[[nodiscard]] uint64_t GenerateU64();
[[nodiscard]] int64_t GenerateI64();
[[nodiscard]] std::string GenerateString(const std::string& prefix);

[[nodiscard]] DsVeosCoSim_DataType GenerateDataType();
[[nodiscard]] DsVeosCoSim_SizeKind GenerateSizeKind();

[[nodiscard]] DsVeosCoSim::IoSignal CreateSignal();
[[nodiscard]] DsVeosCoSim::IoSignal CreateSignal(DsVeosCoSim_DataType dataType);
[[nodiscard]] DsVeosCoSim::IoSignal CreateSignal(DsVeosCoSim_DataType dataType, DsVeosCoSim_SizeKind sizeKind);

[[nodiscard]] std::vector<uint8_t> GenerateIoData(const DsVeosCoSim_IoSignal& signal);
[[nodiscard]] std::vector<uint8_t> CreateZeroedIoData(const DsVeosCoSim_IoSignal& signal);

void FillWithRandom(DsVeosCoSim::CanController& controller);
void FillWithRandom(DsVeosCoSim::EthController& controller);
void FillWithRandom(DsVeosCoSim::LinController& controller);

void FillWithRandom(DsVeosCoSim::CanMessage& message, DsVeosCoSim_BusControllerId controllerId);
void FillWithRandom(DsVeosCoSim::EthMessage& message, DsVeosCoSim_BusControllerId controllerId);
void FillWithRandom(DsVeosCoSim::LinMessage& message, DsVeosCoSim_BusControllerId controllerId);

[[nodiscard]] std::vector<DsVeosCoSim::IoSignal> CreateSignals(size_t count);

[[nodiscard]] std::vector<DsVeosCoSim::CanController> CreateCanControllers(size_t count);
[[nodiscard]] std::vector<DsVeosCoSim::EthController> CreateEthControllers(size_t count);
[[nodiscard]] std::vector<DsVeosCoSim::LinController> CreateLinControllers(size_t count);
