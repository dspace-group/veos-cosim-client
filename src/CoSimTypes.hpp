// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <array>
#include <charconv>
#include <functional>
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

constexpr uint32_t CanMessageMaxLength = 64U;
constexpr uint32_t EthMessageMaxLength = 9018U;
constexpr uint32_t LinMessageMaxLength = 8U;
constexpr uint32_t FrMessageMaxLength = 254U;
constexpr uint32_t EthAddressLength = 6U;

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
struct FrController;
struct FrMessage;
struct FrMessageContainer;
struct SimulationTime;

using SimulationCallback = std::function<void(SimulationTime simulationTime)>;
using SimulationTerminatedCallback = std::function<void(SimulationTime simulationTime, TerminateReason terminateReason)>;
using IncomingSignalChangedCallback = std::function<void(SimulationTime simulationTime, const IoSignal& signal, uint32_t length, const void* value)>;
using CanMessageReceivedCallback = std::function<void(SimulationTime simulationTime, const CanController& canController, const CanMessage& canMessage)>;
using EthMessageReceivedCallback = std::function<void(SimulationTime simulationTime, const EthController& ethController, const EthMessage& ethMessage)>;
using LinMessageReceivedCallback = std::function<void(SimulationTime simulationTime, const LinController& linController, const LinMessage& linMessage)>;
using FrMessageReceivedCallback = std::function<void(SimulationTime simulationTime, const FrController& linController, const FrMessage& frMessage)>;
using CanMessageContainerReceivedCallback =
    std::function<void(SimulationTime simulationTime, const CanController& canController, const CanMessageContainer& canMessageContainer)>;
using EthMessageContainerReceivedCallback =
    std::function<void(SimulationTime simulationTime, const EthController& ethController, const EthMessageContainer& ethMessageContainer)>;
using LinMessageContainerReceivedCallback =
    std::function<void(SimulationTime simulationTime, const LinController& linController, const LinMessageContainer& linMessageContainer)>;
using FrMessageContainerReceivedCallback =
    std::function<void(SimulationTime simulationTime, const FrController& frController, const FrMessageContainer& frMessageContainer)>;

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

ENUM_BITMASK_OPS(CanMessageFlags);

enum class EthMessageFlags : uint32_t {
    Loopback = 1,
    Error = 2,
    Drop = 4
};

ENUM_BITMASK_OPS(EthMessageFlags);

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

enum class FrMessageFlags : uint32_t {
    Loopback = 1,
    Error = 2,
    Drop = 4,
    Startup = 8,
    SyncFrame = 16,
    NullFrame = 32,
    PayloadPreamble = 64,
    TransferOnce = 128,
    ChannelA = 256,
    ChannelB = 512,
};

ENUM_BITMASK_OPS(FrMessageFlags);

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

struct SimulationTime {
    int64_t nanoseconds{};
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
    FrMessageReceivedCallback frMessageReceivedCallback;
    CanMessageContainerReceivedCallback canMessageContainerReceivedCallback;
    LinMessageContainerReceivedCallback linMessageContainerReceivedCallback;
    EthMessageContainerReceivedCallback ethMessageContainerReceivedCallback;
    FrMessageContainerReceivedCallback frMessageContainerReceivedCallback;
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
};

struct IoSignalContainer {
    IoSignalId id{};
    uint32_t length{};
    DataType dataType{};
    SizeKind sizeKind{};
    std::string name;

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
};

struct CanControllerContainer {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    uint64_t flexibleDataRateBitsPerSecond{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] CanController Convert() const;
};

struct CanMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    CanMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};

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
};

struct EthControllerContainer {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    std::array<uint8_t, EthAddressLength> macAddress{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] EthController Convert() const;
};

struct EthMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t reserved{};
    EthMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};

    void WriteTo(EthMessageContainer& ethMessageContainer) const;
};

struct EthMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t reserved{};
    EthMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, EthMessageMaxLength> data{};

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
};

struct LinControllerContainer {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    LinControllerType type{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] LinController Convert() const;
};

struct LinMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    LinMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};

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

    void WriteTo(LinMessage& linMessage) const;
};

struct FrController {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    const char* name{};
    const char* channelName{};
    const char* clusterName{};
};

struct FrControllerContainer {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] FrController Convert() const;
};

struct FrMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    BusMessageId id{};
    FrMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};

    void WriteTo(FrMessageContainer& frMessageContainer) const;
};

struct FrMessageContainer {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t reserved{};
    BusMessageId id{};
    FrMessageFlags flags{};
    uint32_t length{};
    std::array<uint8_t, FrMessageMaxLength> data{};

    void WriteTo(FrMessage& frMessage) const;
};

[[nodiscard]] std::string format_as(const IoSignal& ioSignal);
[[nodiscard]] std::string format_as(const IoSignalContainer& ioSignal);
[[nodiscard]] std::string format_as(const CanController& canController);
[[nodiscard]] std::string format_as(const CanControllerContainer& canController);
[[nodiscard]] std::string format_as(const CanMessage& canMessage);
[[nodiscard]] std::string format_as(const CanMessageContainer& canMessage);
[[nodiscard]] std::string format_as(const EthController& ethController);
[[nodiscard]] std::string format_as(const EthControllerContainer& ethController);
[[nodiscard]] std::string format_as(const EthMessage& ethMessage);
[[nodiscard]] std::string format_as(const EthMessageContainer& ethMessage);
[[nodiscard]] std::string format_as(const LinController& linController);
[[nodiscard]] std::string format_as(const LinControllerContainer& linController);
[[nodiscard]] std::string format_as(const LinMessage& linMessage);
[[nodiscard]] std::string format_as(const LinMessageContainer& linMessage);
[[nodiscard]] std::string format_as(const FrController& frController);
[[nodiscard]] std::string format_as(const FrControllerContainer& frController);
[[nodiscard]] std::string format_as(const FrMessage& frMessage);
[[nodiscard]] std::string format_as(const FrMessageContainer& frMessage);

[[nodiscard]] std::string format_as(SimulationTime simulationTime);
[[nodiscard]] const char* format_as(CoSimType coSimType);
[[nodiscard]] const char* format_as(ConnectionKind connectionKind);
[[nodiscard]] const char* format_as(Command command);
[[nodiscard]] const char* format_as(TerminateReason terminateReason);
[[nodiscard]] const char* format_as(ConnectionState connectionState);
[[nodiscard]] const char* format_as(SimulationState simulationState);
[[nodiscard]] const char* format_as(Mode mode);
[[nodiscard]] std::string format_as(IoSignalId ioSignalId);
[[nodiscard]] const char* format_as(DataType dataType);
[[nodiscard]] const char* format_as(SizeKind sizeKind);
[[nodiscard]] std::string format_as(BusControllerId busControllerId);
[[nodiscard]] std::string format_as(BusMessageId busMessageId);
[[nodiscard]] const char* format_as(LinControllerType linControllerType);
[[nodiscard]] std::string format_as(CanMessageFlags canMessageFlags);
[[nodiscard]] std::string format_as(EthMessageFlags ethMessageFlags);
[[nodiscard]] std::string format_as(LinMessageFlags linMessageFlags);
[[nodiscard]] std::string format_as(FrMessageFlags frMessageFlags);
[[nodiscard]] const char* format_as(FrameKind frameKind);

[[nodiscard]] std::string format_as(const std::vector<IoSignalContainer>& ioSignalContainers);
[[nodiscard]] std::string format_as(const std::vector<CanControllerContainer>& canControllerContainers);
[[nodiscard]] std::string format_as(const std::vector<EthControllerContainer>& ethControllerContainers);
[[nodiscard]] std::string format_as(const std::vector<LinControllerContainer>& linControllerContainers);
[[nodiscard]] std::string format_as(const std::vector<FrControllerContainer>& frControllerContainers);

[[nodiscard]] bool operator==(SimulationTime first, SimulationTime second);
[[nodiscard]] bool operator!=(SimulationTime first, SimulationTime second);
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
[[nodiscard]] bool operator==(const FrController& first, const FrController& second);
[[nodiscard]] bool operator==(const FrControllerContainer& first, const FrControllerContainer& second);
[[nodiscard]] bool operator==(const FrMessage& first, const FrMessage& second);
[[nodiscard]] bool operator==(const FrMessageContainer& first, const FrMessageContainer& second);

[[nodiscard]] std::vector<IoSignal> Convert(const std::vector<IoSignalContainer>& ioSignalContainers);
[[nodiscard]] std::vector<CanController> Convert(const std::vector<CanControllerContainer>& canControllerContainers);
[[nodiscard]] std::vector<EthController> Convert(const std::vector<EthControllerContainer>& ethControllerContainers);
[[nodiscard]] std::vector<LinController> Convert(const std::vector<LinControllerContainer>& linControllerContainers);
[[nodiscard]] std::vector<FrController> Convert(const std::vector<FrControllerContainer>& frControllerContainers);

[[nodiscard]] size_t GetDataTypeSize(DataType dataType);

[[nodiscard]] std::string ValueToString(DataType dataType, uint32_t length, const void* value);
[[nodiscard]] std::string IoDataToString(const IoSignal& ioSignal, uint32_t length, const void* value);
[[nodiscard]] std::string DataToString(const uint8_t* data, size_t dataLength, char separator);

}  // namespace DsVeosCoSim
