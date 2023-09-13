// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <vector>

#include "CoSimTypes.h"

namespace DsVeosCoSim {

int Random(int min, int max);

void FillWithRandom(uint8_t* data, size_t length);

template <typename T>
T GenerateRandom(T min, T max) {
    return (T)Random((int)min, (int)max);
}

uint8_t GenerateU8();
uint16_t GenerateU16();
uint32_t GenerateU32();
uint64_t GenerateU64();
int64_t GenerateI64();
std::string GenerateString(const std::string& prefix);

void CreateSignal(IoSignalContainer& container);

void CreateController(CanControllerContainer& container);
void CreateController(EthControllerContainer& container);
void CreateController(LinControllerContainer& container);

std::vector<IoSignalContainer> CreateSignals(uint32_t count);

std::vector<CanControllerContainer> CreateCanControllers(uint32_t count);
std::vector<EthControllerContainer> CreateEthControllers(uint32_t count);
std::vector<LinControllerContainer> CreateLinControllers(uint32_t count);

void CreateMessage(uint32_t controllerId, CanMessageContainer& container);
void CreateMessage(uint32_t controllerId, EthMessageContainer& container);
void CreateMessage(uint32_t controllerId, LinMessageContainer& container);

}  // namespace DsVeosCoSim
