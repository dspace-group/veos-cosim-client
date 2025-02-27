// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <memory.h>  // IWYU pragma: keep

#include <array>
#include <chrono>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>  // IWYU pragma: keep

namespace DsVeosCoSim {

// NOLINTBEGIN
#define ENUM_BITMASK_OPS(TEnum)                                                    \
    constexpr TEnum operator&(const TEnum lhs, const TEnum rhs) noexcept {         \
        return static_cast<TEnum>(uint32_t(lhs) & uint32_t(rhs));                  \
    }                                                                              \
                                                                                   \
    constexpr TEnum operator|(const TEnum lhs, const TEnum rhs) noexcept {         \
        return static_cast<TEnum>(uint32_t(lhs) | uint32_t(rhs));                  \
    }                                                                              \
                                                                                   \
    constexpr TEnum operator^(const TEnum lhs, const TEnum rhs) noexcept {         \
        return static_cast<TEnum>(uint32_t(lhs) ^ uint32_t(rhs));                  \
    }                                                                              \
                                                                                   \
    constexpr TEnum operator~(const TEnum lhs) noexcept {                          \
        return static_cast<TEnum>(~uint32_t(lhs));                                 \
    }                                                                              \
                                                                                   \
    constexpr TEnum& operator|=(TEnum& lhs, TEnum rhs) noexcept {                  \
        lhs = lhs | rhs;                                                           \
        return lhs;                                                                \
    }                                                                              \
                                                                                   \
    constexpr bool HasFlag(const TEnum flags, const TEnum testFlag) noexcept {     \
        return (flags & testFlag) == testFlag;                                     \
    }                                                                              \
                                                                                   \
    constexpr TEnum ClearFlag(const TEnum flags, const TEnum clearFlag) noexcept { \
        return flags & ~clearFlag;                                                 \
    }
// NOLINTEND

constexpr uint32_t CanMessageMaxLength = 64U;    // NOLINT
constexpr uint32_t EthMessageMaxLength = 9018U;  // NOLINT
constexpr uint32_t LinMessageMaxLength = 8U;     // NOLINT
constexpr uint32_t EthAddressLength = 6U;

using SimulationTime = std::chrono::nanoseconds;

[[nodiscard]] inline std::string SimulationTimeToString(const SimulationTime simulationTime) {
    const int64_t nanoseconds = simulationTime.count();
    std::string representation = std::to_string(nanoseconds);

    const size_t length = representation.size();
    if (length < 10) {
        const size_t countOfMissingZerosInFront = 10 - length;
        representation.insert(representation.begin(), countOfMissingZerosInFront, '0');
    }

    representation.insert(representation.size() - 9, ".");
    while (representation[representation.size() - 1] == '0') {
        representation.pop_back();
    }

    if (representation[representation.size() - 1] == '.') {
        representation.pop_back();
    }

    return representation;
}

enum class Result : uint32_t {
    Ok,
    Error,
    Empty,
    Full,
    InvalidArgument,
    Disconnected
};

[[nodiscard]] inline std::string ToString(const Result result) {
    switch (result) {
        case Result::Ok:
            return "Ok";
        case Result::Error:
            return "Error";
        case Result::Empty:
            return "Empty";
        case Result::Full:
            return "Full";
        case Result::InvalidArgument:
            return "InvalidArgument";
        case Result::Disconnected:
            return "Disconnected";
    }

    return "<Invalid Result>";
}

enum class CoSimType : uint32_t {
    Client,
    Server
};

[[nodiscard]] inline std::string ToString(const CoSimType coSimType) {
    switch (coSimType) {
        case CoSimType::Client:
            return "Client";
        case CoSimType::Server:
            return "Server";
    }

    return "<Invalid CoSimType>";
}

enum class ConnectionKind : uint32_t {
    Remote,
    Local
};

[[nodiscard]] inline std::string ToString(const ConnectionKind connectionKind) {
    switch (connectionKind) {
        case ConnectionKind::Remote:
            return "Remote";
        case ConnectionKind::Local:
            return "Local";
    }

    return "<Invalid ConnectionKind>";
}

enum class Command : uint32_t {
    None,
    Step,
    Start,
    Stop,
    Terminate,
    Pause,
    Continue,
    TerminateFinished,
    Ping
};

[[nodiscard]] inline std::string ToString(const Command command) {
    switch (command) {
        case Command::None:
            return "None";
        case Command::Step:
            return "Step";
        case Command::Start:
            return "Start";
        case Command::Stop:
            return "Stop";
        case Command::Terminate:
            return "Terminate";
        case Command::Pause:
            return "Pause";
        case Command::Continue:
            return "Continue";
        case Command::TerminateFinished:
            return "TerminateFinished";
        case Command::Ping:
            return "Ping";
    }

    return "<Invalid Command>";
}

enum class Severity : uint32_t {
    Error,
    Warning,
    Info,
    Trace
};

[[nodiscard]] inline std::string ToString(const Severity severity) {
    switch (severity) {
        case Severity::Error:
            return "Error";
        case Severity::Warning:
            return "Warning";
        case Severity::Info:
            return "Info";
        case Severity::Trace:
            return "Trace";
    }

    return "<Invalid Severity>";
}

enum class TerminateReason : uint32_t {
    Finished,
    Error
};

[[nodiscard]] inline std::string ToString(const TerminateReason terminateReason) {
    switch (terminateReason) {
        case TerminateReason::Finished:
            return "Finished";
        case TerminateReason::Error:
            return "Error";
    }

    return "<Invalid TerminateReason>";
}

enum class ConnectionState : uint32_t {
    Disconnected,
    Connected
};

[[nodiscard]] inline std::string ToString(const ConnectionState connectionState) {
    switch (connectionState) {
        case ConnectionState::Disconnected:
            return "Disconnected";
        case ConnectionState::Connected:
            return "Connected";
    }

    return "<Invalid ConnectionState>";
}

enum class DataType : uint32_t {
    Bool = 1,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Float32,
    Float64
};

[[nodiscard]] inline size_t GetDataTypeSize(const DataType dataType) {
    switch (dataType) {
        case DataType::Bool:
        case DataType::Int8:
        case DataType::UInt8:
            return 1;
        case DataType::Int16:
        case DataType::UInt16:
            return 2;
        case DataType::Int32:
        case DataType::UInt32:
        case DataType::Float32:
            return 4;
        case DataType::Int64:
        case DataType::UInt64:
        case DataType::Float64:
            return 8;
    }

    return 0;
}

[[nodiscard]] inline std::string ToString(const DataType dataType) {
    switch (dataType) {
        case DataType::Bool:
            return "Bool";
        case DataType::Int8:
            return "Int8";
        case DataType::Int16:
            return "Int16";
        case DataType::Int32:
            return "Int32";
        case DataType::Int64:
            return "Int64";
        case DataType::UInt8:
            return "UInt8";
        case DataType::UInt16:
            return "UInt16";
        case DataType::UInt32:
            return "UInt32";
        case DataType::UInt64:
            return "UInt64";
        case DataType::Float32:
            return "Float32";
        case DataType::Float64:
            return "Float64";
    }

    return "<Invalid DataType>";
}

enum class SizeKind : uint32_t {
    Fixed = 1,
    Variable
};

[[nodiscard]] inline std::string ToString(const SizeKind sizeKind) {
    switch (sizeKind) {
        case SizeKind::Fixed:
            return "Fixed";
        case SizeKind::Variable:
            return "Variable";
    }

    return "<Invalid SizeKind>";
}

[[nodiscard]] inline std::string DataTypeValueToString(const DataType dataType,
                                                       const uint32_t index,
                                                       const void* value) {
    switch (dataType) {
        case DataType::Bool:
            return std::to_string(static_cast<const uint8_t*>(value)[index]);
        case DataType::Int8:
            return std::to_string(static_cast<const int8_t*>(value)[index]);
        case DataType::Int16:
            return std::to_string(static_cast<const int16_t*>(value)[index]);
        case DataType::Int32:
            return std::to_string(static_cast<const int32_t*>(value)[index]);
        case DataType::Int64:
            return std::to_string(static_cast<const int64_t*>(value)[index]);
        case DataType::UInt8:
            return std::to_string(static_cast<const uint8_t*>(value)[index]);
        case DataType::UInt16:
            return std::to_string(static_cast<const uint16_t*>(value)[index]);
        case DataType::UInt32:
            return std::to_string(static_cast<const uint32_t*>(value)[index]);
        case DataType::UInt64:
            return std::to_string(static_cast<const uint64_t*>(value)[index]);
        case DataType::Float32:
            return std::to_string(static_cast<const float*>(value)[index]);
        case DataType::Float64:
            return std::to_string(static_cast<const double*>(value)[index]);
    }

    throw std::runtime_error("Invalid data type.");
}

[[nodiscard]] inline std::string ValueToString(const DataType dataType, const uint32_t length, const void* value) {
    std::ostringstream oss;
    for (uint32_t i = 0; i < length; i++) {
        if (i > 0) {
            oss << " ";
        }

        oss << DataTypeValueToString(dataType, i, value);
    }

    return oss.str();
}

enum class SimulationState {
    Unloaded,
    Stopped,
    Running,
    Paused,
    Terminated
};

[[nodiscard]] inline std::string ToString(const SimulationState simulationState) {
    switch (simulationState) {
        case SimulationState::Unloaded:
            return "Unloaded";
        case SimulationState::Stopped:
            return "Stopped";
        case SimulationState::Running:
            return "Running";
        case SimulationState::Paused:
            return "Paused";
        case SimulationState::Terminated:
            return "Terminated";
    }

    return "<Unknown SimulationState>";
}

enum class Mode {
};

[[nodiscard]] inline std::string ToString([[maybe_unused]] const Mode mode) {
    return "<Unused>";
}

[[nodiscard]] inline std::string DataToString(const uint8_t* data, const size_t dataLength, const char separator = 0) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (uint32_t i = 0; i < dataLength; i++) {
        oss << std::setw(2) << static_cast<int32_t>(data[i]);
        if ((i < (dataLength - 1)) && (separator != 0)) {
            oss << separator;
        }
    }

    return oss.str();
}

enum class IoSignalId : uint32_t {
};

[[nodiscard]] inline std::string ToString(const IoSignalId signalId) {
    return std::to_string(static_cast<uint32_t>(signalId));
}

struct IoSignal {
    IoSignalId id{};
    uint32_t length{};
    DataType dataType{};
    SizeKind sizeKind{};
    const char* name{};
};

struct IoSignalContainer {
    IoSignalId id{};
    uint32_t length{};
    DataType dataType{};
    SizeKind sizeKind{};
    std::string name;

    [[nodiscard]] explicit operator IoSignal() const {
        IoSignal signal{};
        signal.id = id;
        signal.length = length;
        signal.dataType = dataType;
        signal.sizeKind = sizeKind;
        signal.name = name.c_str();
        return signal;
    }
};

[[nodiscard]] inline std::string IoDataToString(const IoSignal& ioSignal, const uint32_t length, const void* value) {
    return "IO Data { Id: " + ToString(ioSignal.id) + ", Length: " + std::to_string(length) +
           ", Data: " + ValueToString(ioSignal.dataType, length, value) + " }";
}

[[nodiscard]] inline std::string ToString(const IoSignal& signal) {
    std::string str = "IO Signal { Id: " + ToString(signal.id) + ", Length: " + std::to_string(signal.length) +
                      ", DataType: " + ToString(signal.dataType) + ", SizeKind: " + ToString(signal.sizeKind) +
                      ", Name: \"" + signal.name + "\"}";

    return str;
}

[[nodiscard]] inline std::string ToString(const IoSignalContainer& signal) {
    std::string str = "IO Signal { Id: " + ToString(signal.id) + ", Length: " + std::to_string(signal.length) +
                      ", DataType: " + ToString(signal.dataType) + ", SizeKind: " + ToString(signal.sizeKind) +
                      ", Name: \"" + signal.name + "\" }";

    return str;
}

[[nodiscard]] inline std::string ToString(const std::vector<IoSignalContainer>& signals) {
    std::string str = "[";

    bool first = true;
    for (const IoSignalContainer& signal : signals) {
        str.append(ToString(signal));

        if (first) {
            first = false;
        } else {
            str.append(", ");
        }
    }

    str.append("]");

    return str;
}

[[nodiscard]] inline std::vector<IoSignal> Convert(const std::vector<IoSignalContainer>& signals) {
    std::vector<IoSignal> ioSignals;
    ioSignals.reserve(signals.size());

    for (const auto& signal : signals) {
        ioSignals.push_back(static_cast<IoSignal>(signal));
    }

    return ioSignals;
}

enum class BusControllerId : uint32_t {
};

[[nodiscard]] inline std::string ToString(BusControllerId busControllerId) {
    return std::to_string(static_cast<uint32_t>(busControllerId));
}

enum class BusMessageId : uint32_t {
};

[[nodiscard]] inline std::string ToString(BusMessageId busMessageId) {
    return std::to_string(static_cast<uint32_t>(busMessageId));
}

enum class CanMessageFlags : uint32_t {
    Loopback = 1,
    Error = 2,
    Drop = 4,
    ExtendedId = 8,
    BitRateSwitch = 16,
    FlexibleDataRateFormat = 32
};

ENUM_BITMASK_OPS(CanMessageFlags);

[[nodiscard]] inline std::string ToString(const CanMessageFlags flags) {
    std::string flagsStr;

    if (HasFlag(flags, CanMessageFlags::Loopback)) {
        flagsStr += ",Loopback";
    }

    if (HasFlag(flags, CanMessageFlags::Error)) {
        flagsStr += ",Error";
    }

    if (HasFlag(flags, CanMessageFlags::Drop)) {
        flagsStr += ",Drop";
    }

    if (HasFlag(flags, CanMessageFlags::ExtendedId)) {
        flagsStr += ",ExtendedId";
    }

    if (HasFlag(flags, CanMessageFlags::BitRateSwitch)) {
        flagsStr += ",BitRateSwitch";
    }

    if (HasFlag(flags, CanMessageFlags::FlexibleDataRateFormat)) {
        flagsStr += ",FlexibleDataRateFormat";
    }

    if (!flagsStr.empty()) {
        flagsStr.erase(0, 1);
    }

    return flagsStr;
}

struct CanController {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    uint64_t flexibleDataRateBitsPerSecond{};
    const char* name{};
    const char* channelName{};
    const char* clusterName{};
};

struct CanControllerContainer {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    uint64_t flexibleDataRateBitsPerSecond{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] explicit operator CanController() const {
        CanController controller{};
        controller.id = id;
        controller.queueSize = queueSize;
        controller.bitsPerSecond = bitsPerSecond;
        controller.flexibleDataRateBitsPerSecond = flexibleDataRateBitsPerSecond;
        controller.name = name.c_str();
        controller.channelName = channelName.c_str();
        controller.clusterName = clusterName.c_str();
        return controller;
    }
};

struct CanMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    CanMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};
};

[[nodiscard]] inline std::string ToString(const CanMessage& message) {
    return "CAN Message { Timestamp: " + SimulationTimeToString(message.timestamp) +
           ", ControllerId: " + ToString(message.controllerId) + ", Id: " + ToString(message.id) +
           ", Length: " + std::to_string(message.length) +
           ", Data: " + DataToString(message.data, message.length, '-') + ", Flags: " + ToString(message.flags) + " }";
}

[[nodiscard]] inline std::string ToString(const CanController& controller) {
    return "CAN Controller { Id: " + ToString(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
           ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) +
           ", FlexibleDataRateBitsPerSecond: " + std::to_string(controller.flexibleDataRateBitsPerSecond) +
           ", Name: \"" + controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
           controller.clusterName + "\" }";
}

[[nodiscard]] inline std::string ToString(const CanControllerContainer& controller) {
    return "CAN Controller { Id: " + ToString(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
           ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) +
           ", FlexibleDataRateBitsPerSecond: " + std::to_string(controller.flexibleDataRateBitsPerSecond) +
           ", Name: \"" + controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
           controller.clusterName + "\" }";
}

[[nodiscard]] inline std::string ToString(const std::vector<CanControllerContainer>& controllers) {
    std::string str = "[";

    bool first = true;
    for (const CanControllerContainer& controller : controllers) {
        str.append(ToString(controller));

        if (first) {
            first = false;
        } else {
            str.append(", ");
        }
    }

    str.append("]");

    return str;
}

[[nodiscard]] inline std::vector<CanController> Convert(const std::vector<CanControllerContainer>& controllers) {
    std::vector<CanController> canControllers;
    canControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        canControllers.push_back(static_cast<CanController>(controller));
    }

    return canControllers;
}

enum class EthMessageFlags : uint32_t {
    Loopback = 1,
    Error = 2,
    Drop = 4
};

ENUM_BITMASK_OPS(EthMessageFlags);

[[nodiscard]] inline std::string ToString(const EthMessageFlags flags) {
    std::string flagsStr;

    if (HasFlag(flags, EthMessageFlags::Loopback)) {
        flagsStr += ",Loopback";
    }

    if (HasFlag(flags, EthMessageFlags::Error)) {
        flagsStr += ",Error";
    }

    if (HasFlag(flags, EthMessageFlags::Drop)) {
        flagsStr += ",Drop";
    }

    if (!flagsStr.empty()) {
        flagsStr.erase(0, 1);
    }

    return flagsStr;
}

struct EthController {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    std::array<uint8_t, EthAddressLength> macAddress{};
    const char* name{};
    const char* channelName{};
    const char* clusterName{};
};

struct EthControllerContainer {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    std::array<uint8_t, EthAddressLength> macAddress{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] explicit operator EthController() const {
        EthController controller{};
        controller.id = id;
        controller.queueSize = queueSize;
        controller.bitsPerSecond = bitsPerSecond;
        controller.macAddress = macAddress;
        controller.name = name.c_str();
        controller.channelName = channelName.c_str();
        controller.clusterName = clusterName.c_str();
        return controller;
    }
};

struct EthMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t reserved{};
    EthMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};
};

[[nodiscard]] inline std::string ToString(const EthMessage& message) {
    return "ETH Message { Timestamp: " + SimulationTimeToString(message.timestamp) +
           ", ControllerId: " + ToString(message.controllerId) + ", Length: " + std::to_string(message.length) +
           ", Data: " + DataToString(message.data, message.length, '-') + ", Flags: " + ToString(message.flags) + " }";
}

[[nodiscard]] inline std::string ToString(const EthController& controller) {
    return "ETH Controller { Id: " + ToString(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
           ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) + ", MacAddress: [" +
           DataToString(controller.macAddress.data(), sizeof(controller.macAddress), ':') + "], Name: \"" +
           controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
           controller.clusterName + "\" }";
}

[[nodiscard]] inline std::string ToString(const EthControllerContainer& controller) {
    return "ETH Controller { Id: " + ToString(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
           ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) + ", MacAddress: [" +
           DataToString(controller.macAddress.data(), sizeof(controller.macAddress), ':') + "], Name: \"" +
           controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
           controller.clusterName + "\" }";
}

[[nodiscard]] inline std::string ToString(const std::vector<EthControllerContainer>& controllers) {
    std::string str = "[";

    bool first = true;
    for (const EthControllerContainer& controller : controllers) {
        str.append(ToString(controller));

        if (first) {
            first = false;
        } else {
            str.append(", ");
        }
    }

    str.append("]");

    return str;
}

[[nodiscard]] inline std::vector<EthController> Convert(const std::vector<EthControllerContainer>& controllers) {
    std::vector<EthController> ethControllers;
    ethControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        ethControllers.push_back(static_cast<EthController>(controller));
    }

    return ethControllers;
}

enum class LinControllerType : uint32_t {
    Responder = 1,
    Commander
};

[[nodiscard]] inline std::string ToString(const LinControllerType type) {
    switch (type) {
        case LinControllerType::Responder:
            return "Responder";
        case LinControllerType::Commander:
            return "Commander";
    }

    return "<Invalid LinControllerType>";
}

enum class LinMessageFlags : uint32_t {
    Loopback = 1,
    Error = 2,
    Drop = 4,
    Header = 8,
    Response = 16,
    WakeEvent = 32,
    SleepEvent = 64,
    EnhancedChecksum = 128,
    TransferOnce = 256,
    ParityFailure = 512,
    Collision = 1024,
    NoResponse = 2048
};

ENUM_BITMASK_OPS(LinMessageFlags);

[[nodiscard]] inline std::string ToString(const LinMessageFlags flags) {
    std::string flagsStr;

    if (HasFlag(flags, LinMessageFlags::Loopback)) {
        flagsStr += ",Loopback";
    }

    if (HasFlag(flags, LinMessageFlags::Error)) {
        flagsStr += ",Error";
    }

    if (HasFlag(flags, LinMessageFlags::Drop)) {
        flagsStr += ",Drop";
    }

    if (HasFlag(flags, LinMessageFlags::Header)) {
        flagsStr += ",Header";
    }

    if (HasFlag(flags, LinMessageFlags::Response)) {
        flagsStr += ",Response";
    }

    if (HasFlag(flags, LinMessageFlags::WakeEvent)) {
        flagsStr += ",WakeEvent";
    }

    if (HasFlag(flags, LinMessageFlags::SleepEvent)) {
        flagsStr += ",SleepEvent";
    }

    if (HasFlag(flags, LinMessageFlags::EnhancedChecksum)) {
        flagsStr += ",EnhancedChecksum";
    }

    if (HasFlag(flags, LinMessageFlags::TransferOnce)) {
        flagsStr += ",TransferOnce";
    }

    if (HasFlag(flags, LinMessageFlags::ParityFailure)) {
        flagsStr += ",ParityFailure";
    }

    if (HasFlag(flags, LinMessageFlags::Collision)) {
        flagsStr += ",Collision";
    }

    if (HasFlag(flags, LinMessageFlags::NoResponse)) {
        flagsStr += ",NoResponse";
    }

    if (!flagsStr.empty()) {
        flagsStr.erase(0, 1);
    }

    return flagsStr;
}

struct LinController {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    LinControllerType type{};
    const char* name{};
    const char* channelName{};
    const char* clusterName{};
};

struct LinControllerContainer {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    LinControllerType type{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] explicit operator LinController() const {
        LinController controller{};
        controller.id = id;
        controller.queueSize = queueSize;
        controller.bitsPerSecond = bitsPerSecond;
        controller.type = type;
        controller.name = name.c_str();
        controller.channelName = channelName.c_str();
        controller.clusterName = clusterName.c_str();
        return controller;
    }
};

struct LinMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    LinMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};
};

[[nodiscard]] inline std::string ToString(const LinMessage& message) {
    return "LIN Message { Timestamp: " + SimulationTimeToString(message.timestamp) +
           ", ControllerId: " + ToString(message.controllerId) + ", Id: " + ToString(message.id) +
           ", Length: " + std::to_string(message.length) +
           ", Data: " + DataToString(message.data, message.length, '-') + ", Flags: " + ToString(message.flags) + " }";
}

[[nodiscard]] inline std::string ToString(const LinController& controller) {
    return "LIN Controller { Id: " + ToString(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
           ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) + ", Type: " + ToString(controller.type) +
           ", Name: \"" + controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
           controller.clusterName + "\" }";
}

[[nodiscard]] inline std::string ToString(const LinControllerContainer& controller) {
    return "LIN Controller { Id: " + ToString(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
           ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) + ", Type: " + ToString(controller.type) +
           ", Name: \"" + controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
           controller.clusterName + "\" }";
}

[[nodiscard]] inline std::string ToString(const std::vector<LinControllerContainer>& controllers) {
    std::string str = "[";

    bool first = true;
    for (const LinControllerContainer& controller : controllers) {
        str.append(ToString(controller));

        if (first) {
            first = false;
        } else {
            str.append(", ");
        }
    }

    str.append("]");

    return str;
}

[[nodiscard]] inline std::vector<LinController> Convert(const std::vector<LinControllerContainer>& controllers) {
    std::vector<LinController> linControllers;
    linControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        linControllers.push_back(static_cast<LinController>(controller));
    }

    return linControllers;
}

using LogCallback = std::function<void(Severity, std::string_view)>;

using SimulationCallback = std::function<void(SimulationTime simulationTime)>;
using SimulationTerminatedCallback = std::function<void(SimulationTime simulationTime, TerminateReason reason)>;
using IncomingSignalChangedCallback =
    std::function<void(SimulationTime simulationTime, const IoSignal& ioSignal, uint32_t length, const void* value)>;
using CanMessageReceivedCallback =
    std::function<void(SimulationTime simulationTime, const CanController& controller, const CanMessage& message)>;
using EthMessageReceivedCallback =
    std::function<void(SimulationTime simulationTime, const EthController& controller, const EthMessage& message)>;
using LinMessageReceivedCallback =
    std::function<void(SimulationTime simulationTime, const LinController& controller, const LinMessage& message)>;

struct Callbacks {
    SimulationCallback simulationStartedCallback;
    SimulationCallback simulationStoppedCallback;
    SimulationTerminatedCallback simulationTerminatedCallback;
    SimulationCallback simulationPausedCallback;
    SimulationCallback simulationContinuedCallback;
    SimulationCallback simulationBeginStepCallback;
    SimulationCallback simulationEndStepCallback;
    IncomingSignalChangedCallback incomingSignalChangedCallback;
    CanMessageReceivedCallback canMessageReceivedCallback;
    LinMessageReceivedCallback linMessageReceivedCallback;
    EthMessageReceivedCallback ethMessageReceivedCallback;
};

struct ConnectConfig {
    std::string remoteIpAddress;
    std::string serverName;
    std::string clientName;
    uint16_t remotePort{};
    uint16_t localPort{};
};

}  // namespace DsVeosCoSim
