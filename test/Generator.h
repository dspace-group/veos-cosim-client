// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include "BusBuffer.h"
#include "CoSimTypes.h"

namespace DsVeosCoSim {

[[nodiscard]] int Random(int min, int max);

void FillWithRandom(uint8_t* data, size_t length);

template <typename T>
[[nodiscard]] T GenerateRandom(T min, T max) {
    return static_cast<T>(Random(static_cast<int>(min), static_cast<int>(max)));
}

[[nodiscard]] uint8_t GenerateU8();
[[nodiscard]] uint16_t GenerateU16();
[[nodiscard]] uint32_t GenerateU32();
[[nodiscard]] uint64_t GenerateU64();
[[nodiscard]] int64_t GenerateI64();
[[nodiscard]] std::string GenerateString(const std::string& prefix);

[[nodiscard]] IoSignal CreateSignal();

void FillWithRandom(CanController& controller);
void FillWithRandom(EthController& controller);
void FillWithRandom(LinController& controller);

void FillWithRandom(CanMessage& message, DsVeosCoSim_BusControllerId controllerId);
void FillWithRandom(EthMessage& message, DsVeosCoSim_BusControllerId controllerId);
void FillWithRandom(LinMessage& message, DsVeosCoSim_BusControllerId controllerId);

[[nodiscard]] std::vector<IoSignal> CreateSignals(size_t count);

[[nodiscard]] std::vector<CanController> CreateCanControllers(size_t count);
[[nodiscard]] std::vector<EthController> CreateEthControllers(size_t count);
[[nodiscard]] std::vector<LinController> CreateLinControllers(size_t count);

}  // namespace DsVeosCoSim
