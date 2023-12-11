// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <vector>

#include "CoSimTypes.h"

namespace DsVeosCoSim {

int Random(int min, int max);

void FillWithRandom(uint8_t* data, size_t length);

template <typename T>
T GenerateRandom(T min, T max) {
    return static_cast<T>(Random(static_cast<int>(min), static_cast<int>(max)));
}

uint8_t GenerateU8();
uint16_t GenerateU16();
uint32_t GenerateU32();
uint64_t GenerateU64();
int64_t GenerateI64();
std::string GenerateString(const std::string& prefix);

void CreateSignal(IoSignal& signal, uint32_t index);

void CreateController(CanController& controller, uint32_t index);
void CreateController(EthController& controller, uint32_t index);
void CreateController(LinController& controller, uint32_t index);

std::vector<IoSignal> CreateSignals(uint32_t count);

std::vector<CanController> CreateCanControllers(uint32_t count);
std::vector<EthController> CreateEthControllers(uint32_t count);
std::vector<LinController> CreateLinControllers(uint32_t count);

void CreateMessage(BusControllerId controllerId, CanMessageContainer& container);
void CreateMessage(BusControllerId controllerId, EthMessageContainer& container);
void CreateMessage(BusControllerId controllerId, LinMessageContainer& container);

}  // namespace DsVeosCoSim
