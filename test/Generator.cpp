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

void CreateSignal(IoSignal& signal) {
    signal.id = GenerateU32();
    signal.length = GenerateRandom(1U, 10U);
    signal.dataType = GenerateRandom(DataType::Bool, DataType::Float64);
    signal.sizeKind = GenerateRandom(SizeKind::Fixed, SizeKind::Variable);
    signal.name = GenerateString("Signal");
}

void CreateController(CanController& controller) {
    controller.id = GenerateU32();
    controller.queueSize = GenerateRandom(1U, 10U);
    controller.bitsPerSecond = GenerateU32();
    controller.flexibleDataRateBitsPerSecond = GenerateU32();
    controller.name = GenerateString("Controller");
    controller.channelName = GenerateString("Channel");
    controller.clusterName = GenerateString("Cluster");
}

void CreateController(EthController& controller) {
    controller.id = GenerateU32();
    controller.queueSize = GenerateRandom(1U, 10U);
    controller.bitsPerSecond = GenerateU32();
    FillWithRandom(controller.macAddress.data(), EthAddressLength);
    controller.name = GenerateString("Controller");
    controller.channelName = GenerateString("Channel");
    controller.clusterName = GenerateString("Cluster");
}

void CreateController(LinController& controller) {
    controller.id = GenerateU32();
    controller.queueSize = GenerateRandom(1U, 10U);
    controller.bitsPerSecond = GenerateU32();
    controller.type = GenerateRandom(LinControllerType::Responder, LinControllerType::Commander);
    controller.name = GenerateString("Controller");
    controller.channelName = GenerateString("Channel");
    controller.clusterName = GenerateString("Cluster");
}

std::vector<IoSignal> CreateSignals(uint32_t count) {
    std::vector<IoSignal> signals;
    signals.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateSignal(signals[i]);
    }

    return signals;
}

std::vector<CanController> CreateCanControllers(uint32_t count) {
    std::vector<CanController> controllers;
    controllers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateController(controllers[i]);
    }

    return controllers;
}

std::vector<EthController> CreateEthControllers(uint32_t count) {
    std::vector<EthController> controllers;
    controllers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateController(controllers[i]);
    }

    return controllers;
}

std::vector<LinController> CreateLinControllers(uint32_t count) {
    std::vector<LinController> controllers;
    controllers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateController(controllers[i]);
    }

    return controllers;
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
