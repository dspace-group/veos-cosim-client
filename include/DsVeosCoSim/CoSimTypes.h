// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <array>
#include <chrono>
#include <functional>
#include <string>
#include <string_view>

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

constexpr uint32_t CanMessageMaxLength = 64U;
constexpr uint32_t EthMessageMaxLength = 9018U;
constexpr uint32_t LinMessageMaxLength = 8U;
constexpr uint32_t EthAddressLength = 6U;

[[nodiscard]] std::string DataToString(const uint8_t* data, size_t dataLength, char separator);

using SimulationTime = std::chrono::nanoseconds;

[[nodiscard]] std::string SimulationTimeToString(SimulationTime simulationTime);

enum class Result : uint32_t {
    Ok,
    Error,
    Empty,
    Full,
    InvalidArgument,
    Disconnected
};

[[nodiscard]] std::string_view ToString(Result result) noexcept;

enum class CoSimType : uint32_t {
    Client,
    Server
};

[[nodiscard]] std::string_view ToString(CoSimType coSimType) noexcept;

enum class ConnectionKind : uint32_t {
    Remote,
    Local
};

[[nodiscard]] std::string_view ToString(ConnectionKind connectionKind) noexcept;

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

[[nodiscard]] std::string_view ToString(Command command) noexcept;

enum class Severity : uint32_t {
    Error,
    Warning,
    Info,
    Trace
};

[[nodiscard]] std::string_view ToString(Severity severity) noexcept;

enum class TerminateReason : uint32_t {
    Finished,
    Error
};

[[nodiscard]] std::string_view ToString(TerminateReason terminateReason) noexcept;

enum class ConnectionState : uint32_t {
    Disconnected,
    Connected
};

[[nodiscard]] std::string_view ToString(ConnectionState connectionState) noexcept;

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

[[nodiscard]] size_t GetDataTypeSize(DataType dataType) noexcept;

[[nodiscard]] std::string_view ToString(DataType dataType) noexcept;

enum class SizeKind : uint32_t {
    Fixed = 1,
    Variable
};

[[nodiscard]] std::string_view ToString(SizeKind sizeKind) noexcept;

[[nodiscard]] std::string ValueToString(DataType dataType, uint32_t length, const void* value);

enum class SimulationState {
    Unloaded,
    Stopped,
    Running,
    Paused,
    Terminated
};

[[nodiscard]] std::string_view ToString(SimulationState simulationState) noexcept;

enum class Mode {
};

[[nodiscard]] std::string_view ToString(Mode mode) noexcept;

[[nodiscard]] std::string DataToString(uint8_t* data, size_t dataLength, char separator);

enum class IoSignalId : uint32_t {
};

[[nodiscard]] std::string ToString(IoSignalId signalId);

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

    [[nodiscard]] explicit operator IoSignal() const;
};

[[nodiscard]] std::string IoDataToString(const IoSignal& ioSignal, uint32_t length, const void* value);
[[nodiscard]] std::string ToString(const IoSignal& signal);
[[nodiscard]] std::string ToString(const IoSignalContainer& signal);
[[nodiscard]] std::string ToString(const std::vector<IoSignalContainer>& signals);

[[nodiscard]] std::vector<IoSignal> Convert(const std::vector<IoSignalContainer>& signals);

enum class BusControllerId : uint32_t {
};

[[nodiscard]] std::string ToString(BusControllerId busControllerId);

enum class BusMessageId : uint32_t {
};

[[nodiscard]] std::string ToString(BusMessageId busMessageId);

enum class CanMessageFlags : uint32_t {
    Loopback = 1,
    Error = 2,
    Drop = 4,
    ExtendedId = 8,
    BitRateSwitch = 16,
    FlexibleDataRateFormat = 32
};

ENUM_BITMASK_OPS(CanMessageFlags);

[[nodiscard]] std::string ToString(CanMessageFlags flags);

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
};

struct CanMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    CanMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};
};

struct CanMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t controllerIndex{};
    BusMessageId id{};
    CanMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, CanMessageMaxLength> data{};
};

[[nodiscard]] std::string ToString(const CanController& controller);
[[nodiscard]] std::string ToString(const CanControllerContainer& controller);
[[nodiscard]] std::string ToString(const CanMessage& message);
[[nodiscard]] std::string ToString(const CanMessageContainer& message);
[[nodiscard]] std::string ToString(const std::vector<CanControllerContainer>& controllers);

[[nodiscard]] CanController Convert(const CanControllerContainer& controller);
[[nodiscard]] std::vector<CanController> Convert(const std::vector<CanControllerContainer>& controllers);

[[nodiscard]] CanMessageContainer Convert(const CanMessage& message);
[[nodiscard]] CanMessage Convert(const CanMessageContainer& message);

enum class EthMessageFlags : uint32_t {
    Loopback = 1,
    Error = 2,
    Drop = 4
};

ENUM_BITMASK_OPS(EthMessageFlags);

[[nodiscard]] std::string ToString(EthMessageFlags flags);

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
};

struct EthMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t reserved{};
    EthMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};
};

struct EthMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t controllerIndex{};
    EthMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, EthMessageMaxLength> data{};
};

[[nodiscard]] std::string ToString(const EthController& controller);
[[nodiscard]] std::string ToString(const EthControllerContainer& controller);
[[nodiscard]] std::string ToString(const EthMessage& message);
[[nodiscard]] std::string ToString(const EthMessageContainer& message);
[[nodiscard]] std::string ToString(const std::vector<EthControllerContainer>& controllers);

[[nodiscard]] EthController Convert(const EthControllerContainer& controller);
[[nodiscard]] std::vector<EthController> Convert(const std::vector<EthControllerContainer>& controllers);

[[nodiscard]] EthMessageContainer Convert(const EthMessage& message);
[[nodiscard]] EthMessage Convert(const EthMessageContainer& message);

enum class LinControllerType : uint32_t {
    Responder = 1,
    Commander
};

[[nodiscard]] std::string_view ToString(LinControllerType type) noexcept;

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

[[nodiscard]] std::string ToString(LinMessageFlags flags);

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
};

struct LinMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    LinMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};
};

struct LinMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t controllerIndex{};
    BusMessageId id{};
    LinMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, LinMessageMaxLength> data{};
};

[[nodiscard]] std::string ToString(const LinController& controller);
[[nodiscard]] std::string ToString(const LinControllerContainer& controller);
[[nodiscard]] std::string ToString(const LinMessage& message);
[[nodiscard]] std::string ToString(const LinMessageContainer& message);
[[nodiscard]] std::string ToString(const std::vector<LinControllerContainer>& controllers);

[[nodiscard]] LinController Convert(const LinControllerContainer& controller);
[[nodiscard]] std::vector<LinController> Convert(const std::vector<LinControllerContainer>& controllers);

[[nodiscard]] LinMessageContainer Convert(const LinMessage& message);
[[nodiscard]] LinMessage Convert(const LinMessageContainer& message);

using LogCallback = std::function<void(Severity, std::string_view)>;

void SetLogCallback(LogCallback logCallback);

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
