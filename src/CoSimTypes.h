// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

#include "DsVeosCoSim/DsVeosCoSim.h"

namespace DsVeosCoSim {

// NOLINTBEGIN(bugprone-macro-parentheses)
#define DEFINE_BITMASK_OPS(TEnum)                                                       \
    static_assert(std::is_enum_v<TEnum>);                                               \
                                                                                        \
    constexpr TEnum operator&(TEnum lhs, TEnum rhs) noexcept {                          \
        using eint_t = std::underlying_type_t<TEnum>;                                   \
        return static_cast<TEnum>(static_cast<eint_t>(lhs) & static_cast<eint_t>(rhs)); \
    }                                                                                   \
                                                                                        \
    constexpr TEnum operator|(TEnum lhs, TEnum rhs) noexcept {                          \
        using eint_t = std::underlying_type_t<TEnum>;                                   \
        return static_cast<TEnum>(static_cast<eint_t>(lhs) | static_cast<eint_t>(rhs)); \
    }                                                                                   \
                                                                                        \
    constexpr TEnum& operator|=(TEnum& lhs, TEnum rhs) noexcept {                       \
        lhs = lhs | rhs;                                                                \
        return lhs;                                                                     \
    }                                                                                   \
                                                                                        \
    constexpr void setFlag(TEnum& flags, TEnum flag) noexcept {                         \
        flags |= flag;                                                                  \
    }                                                                                   \
                                                                                        \
    constexpr bool hasFlag(TEnum flags, TEnum testFlags) noexcept {                     \
        return (flags & testFlags) == testFlags;                                        \
    }
// NOLINTEND(bugprone-macro-parentheses)

constexpr uint32_t CanMessageMaxLength = DSVEOSCOSIM_CAN_MESSAGE_MAX_LENGTH;
constexpr uint32_t EthMessageMaxLength = DSVEOSCOSIM_ETH_MESSAGE_MAX_LENGTH;
constexpr uint32_t LinMessageMaxLength = DSVEOSCOSIM_LIN_MESSAGE_MAX_LENGTH;
constexpr uint32_t EthAddressLength = DSVEOSCOSIM_ETH_ADDRESS_LENGTH;

using IoSignalId = DsVeosCoSim_IoSignalId;
using SimulationTime = DsVeosCoSim_SimulationTime;

[[nodiscard]] inline double SimulationTimeToSeconds(SimulationTime simulationTime) {
    return DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime);
}

enum class Result {
    Ok = DsVeosCoSim_Result_Ok,
    Error = DsVeosCoSim_Result_Error,
    Empty = DsVeosCoSim_Result_Empty,
    Full = DsVeosCoSim_Result_Full,
    InvalidArgument = DsVeosCoSim_Result_InvalidArgument,
    Disconnected = DsVeosCoSim_Result_Disconnected,
    TryAgain
};

#define CheckResult(expression)      \
    do {                             \
        Result _result = expression; \
        if (_result != Result::Ok) { \
            return _result;          \
        }                            \
    } while (0)

[[nodiscard]] inline std::string ToString(Result result) {
    switch (result) {
        case Result::Ok:
            return "OK";
        case Result::Error:
            return "ERROR";
        case Result::Empty:
            return "EMPTY";
        case Result::Full:
            return "FULL";
        case Result::InvalidArgument:
            return "INVALID ARGUMENT";
        case Result::Disconnected:
            return "DISCONNECTED";
        case Result::TryAgain:
            return "TRY AGAIN";
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            return std::to_string(static_cast<int>(result));
    }
}

enum class Command {
    None = DsVeosCoSim_Command_None,
    Step = DsVeosCoSim_Command_Step,
    Start = DsVeosCoSim_Command_Start,
    Stop = DsVeosCoSim_Command_Stop,
    Terminate = DsVeosCoSim_Command_Terminate,
    Pause = DsVeosCoSim_Command_Pause,
    Continue = DsVeosCoSim_Command_Continue
};

[[nodiscard]] inline std::string ToString(Command command) {
    switch (command) {
        case Command::None:
            return "none";
        case Command::Step:
            return "step";
        case Command::Start:
            return "start";
        case Command::Stop:
            return "stop";
        case Command::Terminate:
            return "terminate";
        case Command::Pause:
            return "pause";
        case Command::Continue:
            return "continue";
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            return std::to_string(static_cast<int>(command));
    }
}

enum class Severity {
    Error = DsVeosCoSim_Severity_Error,
    Warning = DsVeosCoSim_Severity_Warning,
    Info = DsVeosCoSim_Severity_Info,
    Trace = DsVeosCoSim_Severity_Trace
};

[[nodiscard]] inline std::string ToString(Severity severity) {
    switch (severity) {
        case Severity::Error:
            return "ERROR";
        case Severity::Warning:
            return "WARNING";
        case Severity::Info:
            return "INFO";
        case Severity::Trace:
            return "TRACE";
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            return std::to_string(static_cast<int>(severity));
    }
}

enum class TerminateReason {
    Finished = DsVeosCoSim_TerminateReason_Finished,
    Error = DsVeosCoSim_TerminateReason_Error
};

[[nodiscard]] inline std::string ToString(TerminateReason terminateReason) {
    switch (terminateReason) {
        case TerminateReason::Finished:
            return "finished";
        case TerminateReason::Error:
            return "error";
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            return std::to_string(static_cast<int>(terminateReason));
    }
}

enum class ConnectionState {
    Connected = DsVeosCoSim_ConnectionState_Connected,
    Disconnected = DsVeosCoSim_ConnectionState_Disconnected
};

[[nodiscard]] inline std::string ToString(ConnectionState connectionState) {
    switch (connectionState) {
        case ConnectionState::Connected:
            return "connected";
        case ConnectionState::Disconnected:
            return "disconnected";
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            return std::to_string(static_cast<int>(connectionState));
    }
}

enum class DataType {
    Bool = DsVeosCoSim_DataType_Bool,
    Int8 = DsVeosCoSim_DataType_Int8,
    Int16 = DsVeosCoSim_DataType_Int16,
    Int32 = DsVeosCoSim_DataType_Int32,
    Int64 = DsVeosCoSim_DataType_Int64,
    UInt8 = DsVeosCoSim_DataType_UInt8,
    UInt16 = DsVeosCoSim_DataType_UInt16,
    UInt32 = DsVeosCoSim_DataType_UInt32,
    UInt64 = DsVeosCoSim_DataType_UInt64,
    Float32 = DsVeosCoSim_DataType_Float32,
    Float64 = DsVeosCoSim_DataType_Float64
};

[[nodiscard]] inline std::string ToString(DataType dataType) {
    switch (dataType) {
        case DataType::Bool:
            return "bool";
        case DataType::Int8:
            return "int8";
        case DataType::Int16:
            return "int16";
        case DataType::Int32:
            return "int32";
        case DataType::Int64:
            return "int64";
        case DataType::UInt8:
            return "uint8";
        case DataType::UInt16:
            return "uint16";
        case DataType::UInt32:
            return "uint32";
        case DataType::UInt64:
            return "uint64";
        case DataType::Float32:
            return "float32";
        case DataType::Float64:
            return "float64";
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            return std::to_string(static_cast<int>(dataType));
    }
}

[[nodiscard]] inline int GetDataTypeSize(DataType dataType) {
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
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            return 0;
    }
}

enum class SizeKind {
    Fixed = DsVeosCoSim_SizeKind_Fixed,
    Variable = DsVeosCoSim_SizeKind_Variable
};

[[nodiscard]] inline std::string ToString(SizeKind sizeKind) {
    switch (sizeKind) {
        case SizeKind::Fixed:
            return "fixed";
        case SizeKind::Variable:
            return "variable";
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            return std::to_string(static_cast<int>(sizeKind));
    }
}

enum class CanMessageFlags {
    Loopback = DsVeosCoSim_CanMessageFlags_Loopback,
    Error = DsVeosCoSim_CanMessageFlags_Error,
    Drop = DsVeosCoSim_CanMessageFlags_Drop,
    ExtendedId = DsVeosCoSim_CanMessageFlags_ExtendedId,
    BitRateSwitch = DsVeosCoSim_CanMessageFlags_BitRateSwitch,
    FlexibleDataRateFormat = DsVeosCoSim_CanMessageFlags_FlexibleDataRateFormat
};

DEFINE_BITMASK_OPS(CanMessageFlags);

[[nodiscard]] inline std::string ToString(CanMessageFlags flags) {
    std::string flagsStr;

    if (hasFlag(flags, CanMessageFlags::Loopback)) {
        flagsStr += ",LOOPBACK";
    }

    if (hasFlag(flags, CanMessageFlags::Error)) {
        flagsStr += ",ERROR";
    }

    if (hasFlag(flags, CanMessageFlags::Drop)) {
        flagsStr += ",DROP";
    }

    if (hasFlag(flags, CanMessageFlags::ExtendedId)) {
        flagsStr += ",EXTENDED_ID";
    }

    if (hasFlag(flags, CanMessageFlags::BitRateSwitch)) {
        flagsStr += ",BIT_RATE_SWITCH";
    }

    if (hasFlag(flags, CanMessageFlags::FlexibleDataRateFormat)) {
        flagsStr += ",FLEXIBLE_DATA_RATE_FORMAT";
    }

    if (!flagsStr.empty()) {
        flagsStr.erase(0, 1);
    }

    return flagsStr;
}

enum class EthMessageFlags {
    Loopback = DsVeosCoSim_EthMessageFlags_Loopback,
    Error = DsVeosCoSim_EthMessageFlags_Error,
    Drop = DsVeosCoSim_EthMessageFlags_Drop
};

DEFINE_BITMASK_OPS(EthMessageFlags);

[[nodiscard]] inline std::string ToString(EthMessageFlags flags) {
    std::string flagsStr;

    if (hasFlag(flags, EthMessageFlags::Loopback)) {
        flagsStr += ",LOOPBACK";
    }

    if (hasFlag(flags, EthMessageFlags::Error)) {
        flagsStr += ",ERROR";
    }

    if (hasFlag(flags, EthMessageFlags::Drop)) {
        flagsStr += ",DROP";
    }

    if (!flagsStr.empty()) {
        flagsStr.erase(0, 1);
    }

    return flagsStr;
}

enum class LinMessageFlags {
    Loopback = DsVeosCoSim_LinMessageFlags_Loopback,
    Error = DsVeosCoSim_LinMessageFlags_Error,
    Drop = DsVeosCoSim_LinMessageFlags_Drop,
    Header = DsVeosCoSim_LinMessageFlags_Header,
    Response = DsVeosCoSim_LinMessageFlags_Response,
    WakeEvent = DsVeosCoSim_LinMessageFlags_WakeEvent,
    SleepEvent = DsVeosCoSim_LinMessageFlags_SleepEvent,
    EnhancedChecksum = DsVeosCoSim_LinMessageFlags_EnhancedChecksum,
    TransferOnce = DsVeosCoSim_LinMessageFlags_TransferOnce,
    ParityFailure = DsVeosCoSim_LinMessageFlags_ParityFailure,
    Collision = DsVeosCoSim_LinMessageFlags_Collision,
    NoResponse = DsVeosCoSim_LinMessageFlags_NoResponse
};

DEFINE_BITMASK_OPS(LinMessageFlags);

[[nodiscard]] inline std::string ToString(LinMessageFlags flags) {
    std::string flagsStr;

    if (hasFlag(flags, LinMessageFlags::Loopback)) {
        flagsStr += ",LOOPBACK";
    }

    if (hasFlag(flags, LinMessageFlags::Error)) {
        flagsStr += ",ERROR";
    }

    if (hasFlag(flags, LinMessageFlags::Drop)) {
        flagsStr += ",DROP";
    }

    if (hasFlag(flags, LinMessageFlags::Header)) {
        flagsStr += ",HEADER";
    }

    if (hasFlag(flags, LinMessageFlags::Response)) {
        flagsStr += ",RESPONSE";
    }

    if (hasFlag(flags, LinMessageFlags::WakeEvent)) {
        flagsStr += ",WAKE_EVENT";
    }

    if (hasFlag(flags, LinMessageFlags::SleepEvent)) {
        flagsStr += ",SLEEP_EVENT";
    }

    if (hasFlag(flags, LinMessageFlags::EnhancedChecksum)) {
        flagsStr += ",ENHANCED_CHECKSUM";
    }

    if (hasFlag(flags, LinMessageFlags::TransferOnce)) {
        flagsStr += ",TRANSFER_ONCE";
    }

    if (hasFlag(flags, LinMessageFlags::ParityFailure)) {
        flagsStr += ",PARITY_FAILURE";
    }

    if (hasFlag(flags, LinMessageFlags::Collision)) {
        flagsStr += ",COLLISION";
    }

    if (hasFlag(flags, LinMessageFlags::NoResponse)) {
        flagsStr += ",NO_RESPONSE";
    }

    if (!flagsStr.empty()) {
        flagsStr.erase(0, 1);
    }

    return flagsStr;
}

struct IoSignal {
    IoSignalId id{};
    uint32_t length{};
    DataType dataType{};
    SizeKind sizeKind{};
    const char* name{};
};

struct IoSignalContainer {
    IoSignal signal{};
    std::string name;
};

using BusControllerId = DsVeosCoSim_BusControllerId;

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
    CanController controller{};
    std::string name;
    std::string channelName;
    std::string clusterName;
};

struct CanMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint32_t id{};
    CanMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};
};

struct CanMessageContainer {
    CanMessage message{};
    std::vector<uint8_t> data;
};

struct EthController {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    uint8_t macAddress[EthAddressLength]{};
    const char* name{};
    const char* channelName{};
    const char* clusterName{};
};

struct EthControllerContainer {
    EthController controller{};
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
    EthMessage message{};
    std::vector<uint8_t> data;
};

enum class LinControllerType {
    Responder = DsVeosCoSim_LinControllerType_Responder,
    Commander = DsVeosCoSim_LinControllerType_Commander
};

[[nodiscard]] inline std::string ToString(LinControllerType type) {
    switch (type) {
        case LinControllerType::Responder:
            return "responder";
        case LinControllerType::Commander:
            return "commander";
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            return std::to_string(static_cast<int>(type));
    }
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
    LinController controller{};
    std::string name;
    std::string channelName;
    std::string clusterName;
};

struct LinMessage {
    SimulationTime timestamp{};
    BusControllerId controllerId{};
    uint8_t id{};
    LinMessageFlags flags{};
    uint32_t length{};
    const uint8_t* data{};
};

struct LinMessageContainer {
    LinMessage message{};
    std::vector<uint8_t> data;
};

using LogCallback = std::function<void(Severity, std::string_view)>;

using SimulationCallback = std::function<void(SimulationTime simulationTime)>;
using SimulationTerminatedCallback = std::function<void(SimulationTime simulationTime, TerminateReason reason)>;
using IncomingSignalChangedCallback = std::function<void(SimulationTime simulationTime, const IoSignal& ioSignal, uint32_t length, const void* value)>;
using CanMessageReceivedCallback = std::function<void(SimulationTime simulationTime, const CanController& controller, const CanMessage& message)>;
using EthMessageReceivedCallback = std::function<void(SimulationTime simulationTime, const EthController& controller, const EthMessage& message)>;
using LinMessageReceivedCallback = std::function<void(SimulationTime simulationTime, const LinController& controller, const LinMessage& message)>;

struct Callbacks {
    DsVeosCoSim_Callbacks callbacks;
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

using ConnectConfig = DsVeosCoSim_ConnectConfig;

}  // namespace DsVeosCoSim
