// Copyright dSPACE GmbH. All rights reserved.

#include "Generator.h"

#include <fmt/format.h>
#include <string>
#include <string_view>

using namespace DsVeosCoSim;

[[nodiscard]] int32_t Random(int32_t min, int32_t max) {
    static bool first = true;
    if (first) {
        srand(42);
        first = false;
    }

    const int32_t diff = max + 1 - min;

    return min + rand() % diff;
}

void FillWithRandom(uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        data[i] = GenerateU8();
    }
}

[[nodiscard]] uint8_t GenerateU8() {
    return GenerateRandom(static_cast<uint8_t>(0U), static_cast<uint8_t>(UINT8_MAX));
}

[[nodiscard]] uint16_t GenerateU16() {
    return GenerateRandom(static_cast<uint16_t>(0U), static_cast<uint16_t>(UINT16_MAX));
}

[[nodiscard]] uint32_t GenerateU32() {
    return GenerateRandom(0U, 123456789U);
}

[[nodiscard]] uint64_t GenerateU64() {
    return (static_cast<uint64_t>(GenerateU32()) << sizeof(uint32_t)) + static_cast<uint64_t>(GenerateU32());
}

[[nodiscard]] int64_t GenerateI64() {
    return static_cast<int64_t>(GenerateU64());
}

[[nodiscard]] std::string GenerateString(std::string_view prefix) {
    return fmt::format("{}{}", prefix, GenerateU32());
}

[[nodiscard]] DsVeosCoSim_DataType GenerateDataType() {
    return GenerateRandom(DsVeosCoSim_DataType_Bool, DsVeosCoSim_DataType_Float64);
}

[[nodiscard]] DsVeosCoSim_SizeKind GenerateSizeKind() {
    return GenerateRandom(DsVeosCoSim_SizeKind_Fixed, DsVeosCoSim_SizeKind_Variable);
}

[[nodiscard]] IoSignal CreateSignal() {
    return CreateSignal(GenerateDataType(), GenerateSizeKind());
}

[[nodiscard]] IoSignal CreateSignal(DsVeosCoSim_DataType dataType) {
    return CreateSignal(dataType, GenerateSizeKind());
}

[[nodiscard]] IoSignal CreateSignal(DsVeosCoSim_DataType dataType, DsVeosCoSim_SizeKind sizeKind) {
    IoSignal signal{};
    signal.id = GenerateU32();
    signal.length = GenerateRandom(1U, 10U);
    signal.dataType = dataType;
    signal.sizeKind = sizeKind;
    signal.name = GenerateString("Signal名前\xF0\x9F\x98\x80");
    return signal;
}

[[nodiscard]] std::vector<uint8_t> GenerateIoData(const DsVeosCoSim_IoSignal& signal) {
    std::vector<uint8_t> data = CreateZeroedIoData(signal);
    FillWithRandom(data.data(), data.size());
    return data;
}

[[nodiscard]] std::vector<uint8_t> CreateZeroedIoData(const DsVeosCoSim_IoSignal& signal) {
    std::vector<uint8_t> data;
    data.resize(GetDataTypeSize(signal.dataType) * signal.length);
    return data;
}

void FillWithRandom(CanController& controller) {
    controller.id = GenerateU32();
    controller.queueSize = 100;
    controller.bitsPerSecond = GenerateU64();
    controller.flexibleDataRateBitsPerSecond = GenerateU64();
    controller.name = GenerateString("CanController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("CanChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("CanCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(EthController& controller) {
    controller.id = GenerateU32();
    controller.queueSize = 100;
    controller.bitsPerSecond = GenerateU64();
    FillWithRandom(controller.macAddress.data(), EthAddressLength);
    controller.name = GenerateString("EthController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("EthChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("EthCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(LinController& controller) {
    controller.id = GenerateU32();
    controller.queueSize = 100;
    controller.bitsPerSecond = GenerateU64();
    controller.type = GenerateRandom(DsVeosCoSim_LinControllerType_Responder, DsVeosCoSim_LinControllerType_Commander);
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

[[nodiscard]] std::vector<IoSignal> CreateSignals(size_t count) {
    std::vector<IoSignal> signals;

    for (size_t i = 0; i < count; i++) {
        signals.push_back(CreateSignal());
    }

    return signals;
}

[[nodiscard]] std::vector<CanController> CreateCanControllers(size_t count) {
    std::vector<CanController> controllers;

    for (size_t i = 0; i < count; i++) {
        CanController controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}

[[nodiscard]] std::vector<EthController> CreateEthControllers(size_t count) {
    std::vector<EthController> controllers;

    for (size_t i = 0; i < count; i++) {
        EthController controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}

[[nodiscard]] std::vector<LinController> CreateLinControllers(size_t count) {
    std::vector<LinController> controllers;

    for (size_t i = 0; i < count; i++) {
        LinController controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}
