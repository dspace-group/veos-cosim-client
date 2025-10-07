// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <array>
#include <chrono>
#include <functional>
#include <ostream>
#include <string>

namespace DsVeosCoSim {

// NOLINTBEGIN
#define ENUM_BITMASK_OPS(TEnum)                                                             \
    constexpr TEnum operator&(TEnum lhs, TEnum rhs) {                                       \
        return static_cast<TEnum>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); \
    }                                                                                       \
                                                                                            \
    constexpr TEnum operator|(TEnum lhs, TEnum rhs) {                                       \
        return static_cast<TEnum>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); \
    }                                                                                       \
                                                                                            \
    constexpr TEnum operator^(TEnum lhs, TEnum rhs) {                                       \
        return static_cast<TEnum>(static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs)); \
    }                                                                                       \
                                                                                            \
    constexpr TEnum operator~(TEnum lhs) {                                                  \
        return static_cast<TEnum>(~static_cast<uint32_t>(lhs));                             \
    }                                                                                       \
                                                                                            \
    constexpr TEnum& operator|=(TEnum& lhs, TEnum rhs) {                                    \
        lhs = lhs | rhs;                                                                    \
        return lhs;                                                                         \
    }                                                                                       \
                                                                                            \
    constexpr bool HasFlag(TEnum flags, TEnum testFlag) {                                   \
        return (flags & testFlag) == testFlag;                                              \
    }                                                                                       \
                                                                                            \
    constexpr TEnum ClearFlag(TEnum flags, TEnum clearFlag) {                               \
        return flags & ~clearFlag;                                                          \
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

enum class Severity : uint32_t;
enum class TerminateReason : uint32_t;

struct IoSignal;
struct CanController;
struct CanMessage;
struct CanMessageContainer;
struct EthController;
struct EthMessage;
struct EthMessageContainer;
struct LinController;
struct LinMessage;
struct LinMessageContainer;

using SimulationTime = std::chrono::nanoseconds;
using LogCallback = std::function<void(Severity, const std::string&)>;

using SimulationCallback = std::function<void(SimulationTime simulationTime)>;
using SimulationTerminatedCallback =
    std::function<void(SimulationTime simulationTime, TerminateReason terminateReason)>;
using IncomingSignalChangedCallback =
    std::function<void(SimulationTime simulationTime, const IoSignal& signal, uint32_t length, const void* value)>;
using CanMessageReceivedCallback = std::function<
    void(SimulationTime simulationTime, const CanController& canController, const CanMessage& canMessage)>;
using EthMessageReceivedCallback = std::function<
    void(SimulationTime simulationTime, const EthController& ethController, const EthMessage& ethMessage)>;
using LinMessageReceivedCallback = std::function<
    void(SimulationTime simulationTime, const LinController& linController, const LinMessage& linMessage)>;
using CanMessageContainerReceivedCallback = std::function<void(SimulationTime simulationTime,
                                                               const CanController& canController,
                                                               const CanMessageContainer& canMessageContainer)>;
using EthMessageContainerReceivedCallback = std::function<void(SimulationTime simulationTime,
                                                               const EthController& ethController,
                                                               const EthMessageContainer& ethMessageContainer)>;
using LinMessageContainerReceivedCallback = std::function<void(SimulationTime simulationTime,
                                                               const LinController& linController,
                                                               const LinMessageContainer& linMessageContainer)>;

enum class Result : uint32_t {
    Ok,
    Error,
    Empty,
    Full,
    InvalidArgument,
    Disconnected
};

enum class CoSimType : uint32_t {
    Client,
    Server
};

enum class ConnectionKind : uint32_t {
    Remote,
    Local
};

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

enum class Severity : uint32_t {
    Error,
    Warning,
    Info,
    Trace
};

enum class TerminateReason : uint32_t {
    Finished,
    Error
};

enum class ConnectionState : uint32_t {
    Disconnected,
    Connected
};

enum class SimulationState : uint32_t {
    Unloaded,
    Stopped,
    Running,
    Paused,
    Terminated
};

enum class Mode : uint32_t {
};

enum class IoSignalId : uint32_t {
};

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

enum class SizeKind : uint32_t {
    Fixed = 1,
    Variable
};

enum class BusControllerId : uint32_t {
};

enum class BusMessageId : uint32_t {
};

enum class LinControllerType : uint32_t {
    Responder = 1,
    Commander
};

enum class CanMessageFlags : uint32_t {
    Loopback = 1,
    Error = 2,
    Drop = 4,
    ExtendedId = 8,
    BitRateSwitch = 16,
    FlexibleDataRateFormat = 32
};

enum class EthMessageFlags : uint32_t {
    Loopback = 1,
    Error = 2,
    Drop = 4
};

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

enum class FrameKind : uint32_t {
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

struct CanMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    CanMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Result Check() const;
    void WriteTo(CanMessageContainer& canMessageContainer) const;
};

struct CanMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t reserved{};
    BusMessageId id{};
    CanMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, CanMessageMaxLength> data{};

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Result Check() const;
    void WriteTo(CanMessage& canMessage) const;
};

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

struct EthMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t reserved{};
    EthMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Result Check() const;
    void WriteTo(EthMessageContainer& ethMessageContainer) const;
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
    void WriteTo(EthMessage& ethMessage) const;
};

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

struct LinMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    LinMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Result Check() const;
    void WriteTo(LinMessageContainer& linMessageContainer) const;
};

struct LinMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t reserved{};
    BusMessageId id{};
    LinMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, LinMessageMaxLength> data{};

    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Result Check() const;
    void WriteTo(LinMessage& linMessage) const;
};

ENUM_BITMASK_OPS(CanMessageFlags);
ENUM_BITMASK_OPS(EthMessageFlags);
ENUM_BITMASK_OPS(LinMessageFlags);

[[nodiscard]] std::string ToString(SimulationTime simulationTime);
[[nodiscard]] const char* ToString(Result result);
[[nodiscard]] const char* ToString(CoSimType coSimType);
[[nodiscard]] const char* ToString(ConnectionKind connectionKind);
[[nodiscard]] const char* ToString(Command command);
[[nodiscard]] const char* ToString(Severity severity);
[[nodiscard]] const char* ToString(TerminateReason terminateReason);
[[nodiscard]] const char* ToString(ConnectionState connectionState);
[[nodiscard]] const char* ToString(SimulationState simulationState);
[[nodiscard]] const char* ToString(Mode mode);
[[nodiscard]] std::string ToString(IoSignalId ioSignalId);
[[nodiscard]] const char* ToString(DataType dataType);
[[nodiscard]] const char* ToString(SizeKind sizeKind);
[[nodiscard]] std::string ToString(BusControllerId busControllerId);
[[nodiscard]] std::string ToString(BusMessageId busMessageId);
[[nodiscard]] const char* ToString(LinControllerType linControllerType);
[[nodiscard]] std::string ToString(CanMessageFlags canMessageFlags);
[[nodiscard]] std::string ToString(EthMessageFlags ethMessageFlags);
[[nodiscard]] std::string ToString(LinMessageFlags linMessageFlags);
[[nodiscard]] const char* ToString(FrameKind frameKind);

[[nodiscard]] std::string ToString(const std::vector<IoSignalContainer>& ioSignalContainers);
[[nodiscard]] std::string ToString(const std::vector<CanControllerContainer>& canControllerContainers);
[[nodiscard]] std::string ToString(const std::vector<EthControllerContainer>& ethControllerContainers);
[[nodiscard]] std::string ToString(const std::vector<LinControllerContainer>& linControllerContainers);

std::ostream& operator<<(std::ostream& stream, SimulationTime simulationTime);
std::ostream& operator<<(std::ostream& stream, Result result);
std::ostream& operator<<(std::ostream& stream, CoSimType coSimType);
std::ostream& operator<<(std::ostream& stream, ConnectionKind connectionKind);
std::ostream& operator<<(std::ostream& stream, Command command);
std::ostream& operator<<(std::ostream& stream, Severity severity);
std::ostream& operator<<(std::ostream& stream, TerminateReason terminateReason);
std::ostream& operator<<(std::ostream& stream, ConnectionState connectionState);
std::ostream& operator<<(std::ostream& stream, SimulationState simulationState);
std::ostream& operator<<(std::ostream& stream, Mode mode);
std::ostream& operator<<(std::ostream& stream, IoSignalId ioSignalId);
std::ostream& operator<<(std::ostream& stream, DataType dataType);
std::ostream& operator<<(std::ostream& stream, SizeKind sizeKind);
std::ostream& operator<<(std::ostream& stream, BusControllerId busControllerId);
std::ostream& operator<<(std::ostream& stream, BusMessageId busMessageId);
std::ostream& operator<<(std::ostream& stream, LinControllerType linControllerType);
std::ostream& operator<<(std::ostream& stream, CanMessageFlags canMessageFlags);
std::ostream& operator<<(std::ostream& stream, EthMessageFlags ethMessageFlags);
std::ostream& operator<<(std::ostream& stream, LinMessageFlags linMessageFlags);
std::ostream& operator<<(std::ostream& stream, FrameKind frameKind);

std::ostream& operator<<(std::ostream& stream, const IoSignal& ioSignal);
std::ostream& operator<<(std::ostream& stream, const IoSignalContainer& ioSignalContainer);
std::ostream& operator<<(std::ostream& stream, const CanController& canController);
std::ostream& operator<<(std::ostream& stream, const CanControllerContainer& canControllerContainer);
std::ostream& operator<<(std::ostream& stream, const CanMessage& canMessage);
std::ostream& operator<<(std::ostream& stream, const CanMessageContainer& canMessageContainer);
std::ostream& operator<<(std::ostream& stream, const EthController& ethController);
std::ostream& operator<<(std::ostream& stream, const EthControllerContainer& ethControllerContainer);
std::ostream& operator<<(std::ostream& stream, const EthMessage& ethMessage);
std::ostream& operator<<(std::ostream& stream, const EthMessageContainer& ethMessageContainer);
std::ostream& operator<<(std::ostream& stream, const LinController& linController);
std::ostream& operator<<(std::ostream& stream, const LinControllerContainer& linControllerContainer);
std::ostream& operator<<(std::ostream& stream, const LinMessage& linMessage);
std::ostream& operator<<(std::ostream& stream, const LinMessageContainer& linMessageContainer);

std::ostream& operator<<(std::ostream& stream, const std::vector<IoSignalContainer>& ioSignalContainers);
std::ostream& operator<<(std::ostream& stream, const std::vector<CanControllerContainer>& canControllerContainers);
std::ostream& operator<<(std::ostream& stream, const std::vector<EthControllerContainer>& ethControllerContainers);
std::ostream& operator<<(std::ostream& stream, const std::vector<LinControllerContainer>& linControllerContainers);

[[nodiscard]] bool operator==(const IoSignal& first, const IoSignal& second);
[[nodiscard]] bool operator==(const IoSignalContainer& first, const IoSignalContainer& second);
[[nodiscard]] bool operator==(const CanController& first, const CanController& second);
[[nodiscard]] bool operator==(const CanControllerContainer& first, const CanControllerContainer& second);
[[nodiscard]] bool operator==(const CanMessage& first, const CanMessage& second);
[[nodiscard]] bool operator==(const CanMessageContainer& first, const CanMessageContainer& second);
[[nodiscard]] bool operator==(const EthController& first, const EthController& second);
[[nodiscard]] bool operator==(const EthControllerContainer& first, const EthControllerContainer& second);
[[nodiscard]] bool operator==(const EthMessage& first, const EthMessage& second);
[[nodiscard]] bool operator==(const EthMessageContainer& first, const EthMessageContainer& second);
[[nodiscard]] bool operator==(const LinController& first, const LinController& second);
[[nodiscard]] bool operator==(const LinControllerContainer& first, const LinControllerContainer& second);
[[nodiscard]] bool operator==(const LinMessage& first, const LinMessage& second);
[[nodiscard]] bool operator==(const LinMessageContainer& first, const LinMessageContainer& second);

[[nodiscard]] std::vector<IoSignal> Convert(const std::vector<IoSignalContainer>& ioSignalContainers);
[[nodiscard]] std::vector<CanController> Convert(const std::vector<CanControllerContainer>& canControllerContainers);
[[nodiscard]] std::vector<EthController> Convert(const std::vector<EthControllerContainer>& ethControllerContainers);
[[nodiscard]] std::vector<LinController> Convert(const std::vector<LinControllerContainer>& linControllerContainers);

[[nodiscard]] size_t GetDataTypeSize(DataType dataType);

[[nodiscard]] std::string ValueToString(DataType dataType, uint32_t length, const void* value);
[[nodiscard]] std::string IoDataToString(const IoSignal& ioSignal, uint32_t length, const void* value);
[[nodiscard]] std::string DataToString(const uint8_t* data, size_t dataLength, char separator);

void SetLogCallback(LogCallback logCallback);

}  // namespace DsVeosCoSim
