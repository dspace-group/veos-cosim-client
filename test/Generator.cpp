// Copyright dSPACE GmbH. All rights reserved.

#include "Generator.h"

#include <string>

namespace DsVeosCoSim {

int Random(int min, int max) {
    static bool first = true;
    if (first) {
        srand(42);  // NOLINT(cert-msc51-cpp)
        first = false;
    }

    return (int)((int64_t)min + rand() % (((int64_t)max + 1) - (int64_t)min));  // NOLINT(concurrency-mt-unsafe)
}

void FillWithRandom(uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        data[i] = GenerateU8();
    }
}

uint8_t GenerateU8() {
    return (uint8_t)GenerateRandom((uint8_t)0, (uint8_t)UINT8_MAX);
}

uint16_t GenerateU16() {
    return (uint8_t)GenerateRandom((uint16_t)0, (uint16_t)UINT16_MAX);
}

uint32_t GenerateU32() {
    return (uint32_t)GenerateRandom(0, 123456789);
}

uint64_t GenerateU64() {
    return (((uint64_t)GenerateU32() << sizeof(uint32_t)) + (uint64_t)GenerateU32());
}

int64_t GenerateI64() {
    return (int64_t)GenerateU64();
}

std::string GenerateString(const std::string& prefix) {
    return prefix + std::to_string(GenerateU32());
}

void CreateSignal(IoSignalContainer& container) {
    container.name = GenerateString("Signal");
    container.signal.id = GenerateU32();
    container.signal.length = GenerateRandom(1U, 10U);
    container.signal.dataType = GenerateRandom(DataType::Bool, DataType::Float64);
    container.signal.sizeKind = GenerateRandom(SizeKind::Fixed, SizeKind::Variable);
    container.signal.name = container.name.data();
}

void CreateController(CanControllerContainer& container) {
    container.name = GenerateString("Controller");
    container.channelName = GenerateString("Channel");
    container.clusterName = GenerateString("Cluster");
    container.controller.id = GenerateU32();
    container.controller.queueSize = GenerateRandom(1U, 10U);
    container.controller.bitsPerSecond = GenerateU32();
    container.controller.flexibleDataRateBitsPerSecond = GenerateU32();
    container.controller.name = container.name.data();
    container.controller.channelName = container.channelName.data();
    container.controller.clusterName = container.clusterName.data();
}

void CreateController(EthControllerContainer& container) {
    container.name = GenerateString("Controller");
    container.channelName = GenerateString("Channel");
    container.clusterName = GenerateString("Cluster");
    container.controller.id = GenerateU32();
    container.controller.queueSize = GenerateRandom(1U, 10U);
    container.controller.bitsPerSecond = GenerateU32();
    FillWithRandom(container.controller.macAddress, DSVEOSCOSIM_ETH_ADDRESS_LENGTH);
    container.controller.name = container.name.data();
    container.controller.channelName = container.channelName.data();
    container.controller.clusterName = container.clusterName.data();
}

void CreateController(LinControllerContainer& container) {
    container.name = GenerateString("Controller");
    container.channelName = GenerateString("Channel");
    container.clusterName = GenerateString("Cluster");
    container.controller.id = GenerateU32();
    container.controller.queueSize = GenerateRandom(1U, 10U);
    container.controller.bitsPerSecond = GenerateU32();
    container.controller.type = GenerateRandom(LinControllerType::Responder, LinControllerType::Commander);
    container.controller.name = container.name.data();
    container.controller.channelName = container.channelName.data();
    container.controller.clusterName = container.clusterName.data();
}

std::vector<IoSignalContainer> CreateSignals(uint32_t count) {
    std::vector<IoSignalContainer> containers;
    containers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateSignal(containers[i]);
    }

    return containers;
}

std::vector<CanControllerContainer> CreateCanControllers(uint32_t count) {
    std::vector<CanControllerContainer> containers;
    containers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateController(containers[i]);
    }

    return containers;
}

std::vector<EthControllerContainer> CreateEthControllers(uint32_t count) {
    std::vector<EthControllerContainer> containers;
    containers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateController(containers[i]);
    }

    return containers;
}

std::vector<LinControllerContainer> CreateLinControllers(uint32_t count) {
    std::vector<LinControllerContainer> containers;
    containers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateController(containers[i]);
    }

    return containers;
}

void CreateMessage(uint32_t controllerId, CanMessageContainer& container) {
    const uint32_t length = GenerateRandom(1U, CanMessageMaxLength);
    container.data.resize(length);
    FillWithRandom(container.data.data(), length);
    container.message.controllerId = controllerId;
    container.message.id = GenerateU32();
    container.message.timestamp = GenerateI64();
    container.message.length = length;
    container.message.data = container.data.data();
}

void CreateMessage(uint32_t controllerId, EthMessageContainer& container) {
    const uint32_t length = GenerateRandom(1U, EthMessageMaxLength);
    container.data.resize(length);
    FillWithRandom(container.data.data(), length);
    container.message.controllerId = controllerId;
    container.message.timestamp = GenerateI64();
    container.message.length = length;
    container.message.data = container.data.data();
}

void CreateMessage(uint32_t controllerId, LinMessageContainer& container) {
    const uint32_t length = GenerateRandom(1U, LinMessageMaxLength);
    container.data.resize(length);
    FillWithRandom(container.data.data(), length);
    container.message.controllerId = controllerId;
    container.message.id = GenerateU8();
    container.message.timestamp = GenerateI64();
    container.message.length = length;
    container.message.data = container.data.data();
}

}  // namespace DsVeosCoSim
