// Copyright dSPACE GmbH. All rights reserved.

#include "Generator.h"

#include <string>
#include "CoSimTypes.h"

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

IoSignal CreateSignal() {
    IoSignal signal{};
    signal.id = static_cast<IoSignalId>(GenerateU32());
    signal.length = GenerateRandom(1U, 10U);
    signal.dataType = GenerateRandom(DsVeosCoSim_DataType_Bool, DsVeosCoSim_DataType_Float64);
    signal.sizeKind = GenerateRandom(DsVeosCoSim_SizeKind_Fixed, DsVeosCoSim_SizeKind_Variable);
    signal.name = GenerateString("Signal名前\xF0\x9F\x98\x80");
    return signal;
}

void FillWithRandom(CanController& controller) {
    controller.id = GenerateU32();
    controller.queueSize = 100;
    controller.bitsPerSecond = 500000;
    controller.flexibleDataRateBitsPerSecond = 2000000;
    controller.name = GenerateString("CanController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("CanChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("CanCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(EthController& controller) {
    controller.id = GenerateU32();
    controller.queueSize = 100;
    controller.bitsPerSecond = 1000000000;
    FillWithRandom(controller.macAddress.data(), EthAddressLength);
    controller.name = GenerateString("EthController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("EthChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("EthCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(LinController& controller) {
    controller.id = GenerateU32();
    controller.queueSize = 100;
    controller.bitsPerSecond = 19200;
    controller.type = GenerateRandom(LinControllerType::Responder, LinControllerType::Commander);
    controller.name = GenerateString("LinController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("LinChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("LinCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(CanMessage& message, DsVeosCoSim_BusControllerId controllerId) {
    const uint32_t length = GenerateRandom(1U, CanMessageMaxLength);
    message.controllerId = controllerId;
    message.id = GenerateU32();
    message.timestamp = GenerateI64();
    message.length = length;
    FillWithRandom(message.data.data(), length);
}

void FillWithRandom(EthMessage& message, DsVeosCoSim_BusControllerId controllerId) {
    const uint32_t length = GenerateRandom(1U, EthMessageMaxLength);
    message.controllerId = controllerId;
    message.timestamp = GenerateI64();
    message.length = length;
    FillWithRandom(message.data.data(), length);
}

void FillWithRandom(LinMessage& message, DsVeosCoSim_BusControllerId controllerId) {
    const uint32_t length = GenerateRandom(1U, LinMessageMaxLength);
    message.controllerId = controllerId;
    message.id = GenerateU32();
    message.timestamp = GenerateI64();
    message.length = length;
    FillWithRandom(message.data.data(), length);
}

std::vector<IoSignal> CreateSignals(size_t count) {
    std::vector<IoSignal> signals;

    for (size_t i = 0; i < count; i++) {
        signals.push_back(CreateSignal());
    }

    return signals;
}

std::vector<CanController> CreateCanControllers(size_t count) {
    std::vector<CanController> controllers;

    for (size_t i = 0; i < count; i++) {
        CanController controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}

std::vector<EthController> CreateEthControllers(size_t count) {
    std::vector<EthController> controllers;

    for (size_t i = 0; i < count; i++) {
        EthController controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}

std::vector<LinController> CreateLinControllers(size_t count) {
    std::vector<LinController> controllers;

    for (size_t i = 0; i < count; i++) {
        LinController controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}

}  // namespace DsVeosCoSim
