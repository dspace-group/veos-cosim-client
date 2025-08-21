// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <array>
#include <chrono>
#include <functional>
#include <string>

namespace DsVeosCoSim {

// NOLINTBEGIN
#define ENUM_BITMASK_OPS(TEnum)                                   \
    constexpr TEnum operator&(TEnum lhs, TEnum rhs) {             \
        return static_cast<TEnum>(uint32_t(lhs) & uint32_t(rhs)); \
    }                                                             \
                                                                  \
    constexpr TEnum operator|(TEnum lhs, TEnum rhs) {             \
        return static_cast<TEnum>(uint32_t(lhs) | uint32_t(rhs)); \
    }                                                             \
                                                                  \
    constexpr TEnum operator^(TEnum lhs, TEnum rhs) {             \
        return static_cast<TEnum>(uint32_t(lhs) ^ uint32_t(rhs)); \
    }                                                             \
                                                                  \
    constexpr TEnum operator~(TEnum lhs) {                        \
        return static_cast<TEnum>(~uint32_t(lhs));                \
    }                                                             \
                                                                  \
    constexpr TEnum& operator|=(TEnum& lhs, TEnum rhs) {          \
        lhs = lhs | rhs;                                          \
        return lhs;                                               \
    }                                                             \
                                                                  \
    constexpr bool HasFlag(TEnum flags, TEnum testFlag) {         \
        return (flags & testFlag) == testFlag;                    \
    }                                                             \
                                                                  \
    constexpr TEnum ClearFlag(TEnum flags, TEnum clearFlag) {     \
        return flags & ~clearFlag;                                \
    }
// NOLINTEND

#define IsOk(result) ((result) == Result::Ok)
#define IsDisconnected(result) ((result) == Result::Disconnected)

#define CheckResult(result)         \
    do {                            \
        Result _result_ = (result); \
        if (!IsOk(_result_)) {      \
            return _result_;        \
        }                           \
    } while (0)

#define CheckBoolResult(result)   \
    do {                          \
        if (!(result)) {          \
            return Result::Error; \
        }                         \
    } while (0)

constexpr uint32_t CanMessageMaxLength = 64U;
constexpr uint32_t EthMessageMaxLength = 9018U;
constexpr uint32_t LinMessageMaxLength = 8U;
constexpr uint32_t EthAddressLength = 6U;

[[nodiscard]] std::string DataToString(const uint8_t* data, size_t dataLength, char separator);

using SimulationTime = std::chrono::nanoseconds;

[[nodiscard]] std::string SimulationTimeToString(SimulationTime simulationTime);

enum class Result {
    Ok,
    Error,
    Empty,
    Full,
    InvalidArgument,
    Disconnected
};

[[nodiscard]] const char* ToString(Result result);

enum class CoSimType : uint32_t {
    Client,
    Server
};

[[nodiscard]] const char* ToString(CoSimType coSimType);

enum class ConnectionKind : uint32_t {
    Remote,
    Local
};

[[nodiscard]] const char* ToString(ConnectionKind connectionKind);

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

[[nodiscard]] const char* ToString(Command command);

enum class Severity : uint32_t {
    Error,
    Warning,
    Info,
    Trace
};

[[nodiscard]] const char* ToString(Severity severity);

enum class TerminateReason : uint32_t {
    Finished,
    Error
};

[[nodiscard]] const char* ToString(TerminateReason terminateReason);

enum class ConnectionState : uint32_t {
    Disconnected,
    Connected
};

[[nodiscard]] const char* ToString(ConnectionState connectionState);

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

[[nodiscard]] size_t GetDataTypeSize(DataType dataType);

[[nodiscard]] const char* ToString(DataType dataType);

enum class SizeKind : uint32_t {
    Fixed = 1,
    Variable
};

[[nodiscard]] const char* ToString(SizeKind sizeKind);

[[nodiscard]] std::string ValueToString(DataType dataType, uint32_t length, const void* value);

enum class SimulationState {
    Unloaded,
    Stopped,
    Running,
    Paused,
    Terminated
};

[[nodiscard]] const char* ToString(SimulationState simulationState);

enum class Mode {
};

[[nodiscard]] const char* ToString(Mode mode);

enum class IoSignalId : uint32_t {
};

[[nodiscard]] std::string ToString(IoSignalId signalId);

struct IoSignal {
    IoSignalId id{};
    uint32_t length{};
    DataType dataType{};
    SizeKind sizeKind{};
    const char* name{};

    [[nodiscard]] std::string ToString() const;
};

struct IoSignalContainer {
    IoSignalId id{};
    uint32_t length{};
    DataType dataType{};
    SizeKind sizeKind{};
    std::string name;

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] IoSignal Convert() const;
};

[[nodiscard]] std::string IoDataToString(const IoSignal& ioSignal, uint32_t length, const void* value);
[[nodiscard]] std::string ToString(const std::vector<IoSignalContainer>& signalContainers);

[[nodiscard]] std::vector<IoSignal> Convert(const std::vector<IoSignalContainer>& signalContainers);

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

    [[nodiscard]] std::string ToString() const;
};

struct CanControllerContainer {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    uint64_t flexibleDataRateBitsPerSecond{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] CanController Convert() const;
};

[[nodiscard]] std::string ToString(const std::vector<CanControllerContainer>& controllerContainers);

[[nodiscard]] std::vector<CanController> Convert(const std::vector<CanControllerContainer>& controllerContainers);

struct CanMessageContainer;

struct CanMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    CanMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Result Check() const;
    void WriteTo(CanMessageContainer& messageContainer) const;
};

struct CanMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    CanMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, CanMessageMaxLength> data{};

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Result Check() const;
    void WriteTo(CanMessage& message) const;
};

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

    [[nodiscard]] std::string ToString() const;
};

struct EthControllerContainer {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    std::array<uint8_t, EthAddressLength> macAddress{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] EthController Convert() const;
};

[[nodiscard]] std::string ToString(const std::vector<EthControllerContainer>& controllerContainers);

[[nodiscard]] std::vector<EthController> Convert(const std::vector<EthControllerContainer>& controllerContainers);

struct EthMessageContainer;

struct EthMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t reserved{};
    EthMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Result Check() const;
    void WriteTo(EthMessageContainer& messageContainer) const;
};

struct EthMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t reserved{};
    EthMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, EthMessageMaxLength> data{};

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Result Check() const;
    void WriteTo(EthMessage& message) const;
};

enum class LinControllerType : uint32_t {
    Responder = 1,
    Commander
};

[[nodiscard]] const char* ToString(LinControllerType type);

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

    [[nodiscard]] std::string ToString() const;
};

struct LinControllerContainer {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    LinControllerType type{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] LinController Convert() const;
};

[[nodiscard]] std::string ToString(const std::vector<LinControllerContainer>& controllerContainers);

[[nodiscard]] std::vector<LinController> Convert(const std::vector<LinControllerContainer>& controllerContainers);

struct LinMessageContainer;

struct LinMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    LinMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Result Check() const;
    void WriteTo(LinMessageContainer& messageContainer) const;
};

struct LinMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    LinMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, LinMessageMaxLength> data{};

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Result Check() const;
    void WriteTo(LinMessage& message) const;
};

using LogCallback = std::function<void(Severity, const std::string&)>;

void SetLogCallback(LogCallback logCallback);

using SimulationCallback = std::function<void(SimulationTime simulationTime)>;
using SimulationTerminatedCallback = std::function<void(SimulationTime simulationTime, TerminateReason reason)>;
using IncomingSignalChangedCallback =
    std::function<void(SimulationTime simulationTime, const IoSignal& signal, uint32_t length, const void* value)>;
using CanMessageReceivedCallback =
    std::function<void(SimulationTime simulationTime, const CanController& controller, const CanMessage& message)>;
using EthMessageReceivedCallback =
    std::function<void(SimulationTime simulationTime, const EthController& controller, const EthMessage& message)>;
using LinMessageReceivedCallback =
    std::function<void(SimulationTime simulationTime, const LinController& controller, const LinMessage& message)>;
using CanMessageContainerReceivedCallback = std::function<
    void(SimulationTime simulationTime, const CanController& controller, const CanMessageContainer& messageContainer)>;
using EthMessageContainerReceivedCallback = std::function<
    void(SimulationTime simulationTime, const EthController& controller, const EthMessageContainer& messageContainer)>;
using LinMessageContainerReceivedCallback = std::function<
    void(SimulationTime simulationTime, const LinController& controller, const LinMessageContainer& messageContainer)>;

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
    CanMessageContainerReceivedCallback canMessageContainerReceivedCallback;
    LinMessageContainerReceivedCallback linMessageContainerReceivedCallback;
    EthMessageContainerReceivedCallback ethMessageContainerReceivedCallback;
};

struct ConnectConfig {
    std::string remoteIpAddress;
    std::string serverName;
    std::string clientName;
    uint16_t remotePort{};
    uint16_t localPort{};
};

enum class FrameKind {
    Ok = 1,
    Error,

    Connect,
    ConnectOk,

    Ping,
    PingOk,

    Start,
    Stop,
    Terminate,
    Pause,
    Continue,

    Step,
    StepOk,

    GetPort,
    GetPortOk,
    SetPort,
    UnsetPort
};

[[nodiscard]] const char* ToString(FrameKind frameKind);

}  // namespace DsVeosCoSim
