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

    const int diff = max + 1 - min;

    return min + rand() % diff;  // NOLINT(concurrency-mt-unsafe)
}

void FillWithRandom(uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        data[i] = GenerateU8();
    }
}

uint8_t GenerateU8() {
    return GenerateRandom(static_cast<uint8_t>(0U), static_cast<uint8_t>(UINT8_MAX));
}

uint16_t GenerateU16() {
    return GenerateRandom(static_cast<uint16_t>(0U), static_cast<uint16_t>(UINT16_MAX));
}

uint32_t GenerateU32() {
    return GenerateRandom(0U, 123456789U);
}

uint64_t GenerateU64() {
    return (static_cast<uint64_t>(GenerateU32()) << sizeof(uint32_t)) + static_cast<uint64_t>(GenerateU32());
}

int64_t GenerateI64() {
    return static_cast<int64_t>(GenerateU64());
}

std::string GenerateString(const std::string& prefix) {
    return prefix + std::to_string(GenerateU32());
}

void CreateSignal(IoSignal& signal, uint32_t index) {
    signal.id = static_cast<IoSignalId>(GenerateU32());
    signal.length = GenerateRandom(1U, 10U);
    signal.dataType = GenerateRandom(DataType::Bool, DataType::Float64);
    signal.sizeKind = GenerateRandom(SizeKind::Fixed, SizeKind::Variable);
    signal.name = "Signal日本語" + std::to_string(index);
}

void CreateController(CanController& controller, uint32_t index) {
    controller.id = static_cast<BusControllerId>(GenerateU32());
    controller.queueSize = 100;
    controller.bitsPerSecond = 500000;
    controller.flexibleDataRateBitsPerSecond = 2000000;
    controller.name = "CanController日本語" + std::to_string(index);
    controller.channelName = GenerateString("Channel日本語");
    controller.clusterName = GenerateString("Cluster日本語");
}

void CreateController(EthController& controller, uint32_t index) {
    controller.id = static_cast<BusControllerId>(GenerateU32());
    controller.queueSize = 100;
    controller.bitsPerSecond = 1000000000;
    FillWithRandom(controller.macAddress.data(), EthAddressLength);
    controller.name = "EthController日本語" + std::to_string(index);
    controller.channelName = GenerateString("Channel日本語");
    controller.clusterName = GenerateString("Cluster日本語");
}

void CreateController(LinController& controller, uint32_t index) {
    controller.id = static_cast<BusControllerId>(GenerateU32());
    controller.queueSize = 100;
    controller.bitsPerSecond = 19200;
    controller.type = GenerateRandom(LinControllerType::Responder, LinControllerType::Commander);
    controller.name = "LinController日本語" + std::to_string(index);
    controller.channelName = GenerateString("Channel日本語");
    controller.clusterName = GenerateString("Cluster日本語");
}

std::vector<IoSignal> CreateSignals(uint32_t count) {
    std::vector<IoSignal> signals;
    signals.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateSignal(signals[i], i);
    }

    return signals;
}

std::vector<CanController> CreateCanControllers(uint32_t count) {
    std::vector<CanController> controllers;
    controllers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateController(controllers[i], i);
    }

    return controllers;
}

std::vector<EthController> CreateEthControllers(uint32_t count) {
    std::vector<EthController> controllers;
    controllers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateController(controllers[i], i);
    }

    return controllers;
}

std::vector<LinController> CreateLinControllers(uint32_t count) {
    std::vector<LinController> controllers;
    controllers.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        CreateController(controllers[i], i);
    }

    return controllers;
}

void CreateMessage(BusControllerId controllerId, CanMessageContainer& container) {
    const uint32_t length = GenerateRandom(1U, CanMessageMaxLength);
    FillWithRandom(container.data.data(), length);
    container.message.controllerId = controllerId;
    container.message.id = GenerateU32();
    container.message.timestamp = GenerateI64();
    container.message.length = length;
    container.message.data = container.data.data();
}

void CreateMessage(BusControllerId controllerId, EthMessageContainer& container) {
    const uint32_t length = GenerateRandom(1U, EthMessageMaxLength);
    FillWithRandom(container.data.data(), length);
    container.message.controllerId = controllerId;
    container.message.timestamp = GenerateI64();
    container.message.length = length;
    container.message.data = container.data.data();
}

void CreateMessage(BusControllerId controllerId, LinMessageContainer& container) {
    const uint32_t length = GenerateRandom(1U, LinMessageMaxLength);
    FillWithRandom(container.data.data(), length);
    container.message.controllerId = controllerId;
    container.message.id = GenerateU8();
    container.message.timestamp = GenerateI64();
    container.message.length = length;
    container.message.data = container.data.data();
}

}  // namespace DsVeosCoSim
