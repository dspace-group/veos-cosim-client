// Copyright dSPACE GmbH. All rights reserved.

#include "Generator.h"

#include <fmt/format.h>

#include <string>
#include <string_view>

#include "DsVeosCoSim/CoSimTypes.h"

using namespace std::chrono;
using namespace DsVeosCoSim;

namespace {

[[nodiscard]] DataType GenerateDataType() {
    return GenerateRandom(DataType::Bool, DataType::Float64);
}

[[nodiscard]] SizeKind GenerateSizeKind() {
    return GenerateRandom(SizeKind::Fixed, SizeKind::Variable);
}

[[nodiscard]] BusControllerId GenerateBusControllerId() {
    return static_cast<BusControllerId>(GenerateU32());
}

[[nodiscard]] BusMessageId GenerateBusMessageId() {
    return static_cast<BusMessageId>(GenerateU32());
}

}  // namespace

[[nodiscard]] int32_t Random(int32_t min, int32_t max) {
    static bool first = true;
    if (first) {
        srand(static_cast<uint32_t>(std::time({})));  // NOLINT(cert-msc51-cpp, cert-msc32-c)
        first = false;
    }

    int32_t diff = max + 1 - min;

    return min + (rand() % diff);  // NOLINT(concurrency-mt-unsafe, cert-msc30-c, cert-msc50-cpp)
}

void FillWithRandom(uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        data[i] = GenerateU8();
    }
}

[[nodiscard]] uint8_t GenerateU8() {
    return GenerateRandom(static_cast<uint8_t>(0U), static_cast<uint8_t>(255U));
}

[[nodiscard]] uint16_t GenerateU16() {
    return GenerateRandom(static_cast<uint16_t>(0U), static_cast<uint16_t>(65535U));
}

[[nodiscard]] uint32_t GenerateU32() {
    return GenerateRandom(0U, 123456789U);
}

[[nodiscard]] uint32_t GenerateU32(uint32_t min, uint32_t max) {
    return static_cast<uint32_t>(Random(static_cast<int32_t>(min), static_cast<int32_t>(max)));
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

[[nodiscard]] SimulationTime GenerateSimulationTime() {
    return SimulationTime(GenerateU64());
}

[[nodiscard]] BusMessageId GenerateBusMessageId(uint32_t min, uint32_t max) {
    return static_cast<BusMessageId>(GenerateU32(min, max));
}

[[nodiscard]] std::vector<uint8_t> GenerateBytes(size_t length) {
    std::vector<uint8_t> data;
    data.resize(length);
    for (size_t i = 0; i < length; i++) {
        data[i] = GenerateU8();
    }

    return data;
}

[[nodiscard]] IoSignalContainer CreateSignal() {
    return CreateSignal(GenerateDataType(), GenerateSizeKind());
}

[[nodiscard]] IoSignalContainer CreateSignal(DataType dataType) {
    return CreateSignal(dataType, GenerateSizeKind());
}

[[nodiscard]] IoSignalContainer CreateSignal(DataType dataType, SizeKind sizeKind) {
    IoSignalContainer signal{};
    signal.id = static_cast<IoSignalId>(GenerateU32());
    signal.length = GenerateRandom(1U, 4U);
    signal.dataType = dataType;
    signal.sizeKind = sizeKind;
    signal.name = GenerateString("Signal名前\xF0\x9F\x98\x80");
    return signal;
}

[[nodiscard]] std::vector<uint8_t> GenerateIoData(const IoSignalContainer& signal) {
    std::vector<uint8_t> data = CreateZeroedIoData(signal);
    FillWithRandom(data.data(), data.size());
    return data;
}

[[nodiscard]] std::vector<uint8_t> CreateZeroedIoData(const IoSignalContainer& signal) {
    std::vector<uint8_t> data;
    data.resize(GetDataTypeSize(signal.dataType) * signal.length);
    return data;
}

void FillWithRandom(CanControllerContainer& controller) {
    controller.id = GenerateBusControllerId();
    controller.queueSize = 100;
    controller.bitsPerSecond = GenerateU64();
    controller.flexibleDataRateBitsPerSecond = GenerateU64();
    controller.name = GenerateString("CanController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("CanChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("CanCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(EthControllerContainer& controller) {
    controller.id = GenerateBusControllerId();
    controller.queueSize = 100;
    controller.bitsPerSecond = GenerateU64();
    FillWithRandom(controller.macAddress.data(), EthAddressLength);
    controller.name = GenerateString("EthController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("EthChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("EthCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(LinControllerContainer& controller) {
    controller.id = GenerateBusControllerId();
    controller.queueSize = 100;
    controller.bitsPerSecond = GenerateU64();
    controller.type = GenerateRandom(LinControllerType::Responder, LinControllerType::Commander);
    controller.name = GenerateString("LinController名前\xF0\x9F\x98\x80");
    controller.channelName = GenerateString("LinChannel名前\xF0\x9F\x98\x80");
    controller.clusterName = GenerateString("LinCluster名前\xF0\x9F\x98\x80");
}

void FillWithRandom(CanMessageContainer& message, BusControllerId controllerId) {
    uint32_t length = GenerateRandom(1U, 8U);
    message.controllerId = controllerId;
    message.id = GenerateBusMessageId();
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    FillWithRandom(message.data.data(), length);
}

void FillWithRandom(EthMessageContainer& message, BusControllerId controllerId) {
    uint32_t length = GenerateRandom(1U, 8U);
    message.controllerId = controllerId;
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    FillWithRandom(message.data.data(), length);
}

void FillWithRandom(LinMessageContainer& message, BusControllerId controllerId) {
    uint32_t length = GenerateRandom(1U, 8U);
    message.controllerId = controllerId;
    message.id = GenerateBusMessageId();
    message.timestamp = GenerateSimulationTime();
    message.length = length;
    FillWithRandom(message.data.data(), length);
}

[[nodiscard]] std::vector<IoSignalContainer> CreateSignals(size_t count) {
    std::vector<IoSignalContainer> signals;
    signals.reserve(count);

    for (size_t i = 0; i < count; i++) {
        signals.push_back(CreateSignal());
    }

    return signals;
}

[[nodiscard]] std::vector<CanControllerContainer> CreateCanControllers(size_t count) {
    std::vector<CanControllerContainer> controllers;

    for (size_t i = 0; i < count; i++) {
        CanControllerContainer controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}

[[nodiscard]] std::vector<EthControllerContainer> CreateEthControllers(size_t count) {
    std::vector<EthControllerContainer> controllers;

    for (size_t i = 0; i < count; i++) {
        EthControllerContainer controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}

[[nodiscard]] std::vector<LinControllerContainer> CreateLinControllers(size_t count) {
    std::vector<LinControllerContainer> controllers;

    for (size_t i = 0; i < count; i++) {
        LinControllerContainer controller{};
        FillWithRandom(controller);
        controllers.push_back(controller);
    }

    return controllers;
}
