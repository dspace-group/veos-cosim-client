// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <array>
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
        case Result::TryAgain:
            return "TryAgain";
    }

    return std::to_string(static_cast<int>(result));
}

[[nodiscard]] inline std::string ToString(bool value) {
    return value ? "true" : "false";
}

enum class Command {
    None = DsVeosCoSim_Command_None,
    Step = DsVeosCoSim_Command_Step,
    Start = DsVeosCoSim_Command_Start,
    Stop = DsVeosCoSim_Command_Stop,
    Terminate = DsVeosCoSim_Command_Terminate,
    Pause = DsVeosCoSim_Command_Pause,
    Continue = DsVeosCoSim_Command_Continue,
    TerminateFinished,
    Ping
};

[[nodiscard]] inline std::string ToString(Command command) {
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

    return std::to_string(static_cast<int>(command));
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
            return "Error";
        case Severity::Warning:
            return "Warning";
        case Severity::Info:
            return "Info";
        case Severity::Trace:
            return "Trace";
    }

    return std::to_string(static_cast<int>(severity));
}

enum class TerminateReason {
    Finished = DsVeosCoSim_TerminateReason_Finished,
    Error = DsVeosCoSim_TerminateReason_Error
};

[[nodiscard]] inline std::string ToString(TerminateReason terminateReason) {
    switch (terminateReason) {
        case TerminateReason::Finished:
            return "Finished";
        case TerminateReason::Error:
            return "Error";
    }

    return std::to_string(static_cast<int>(terminateReason));
}

enum class ConnectionState {
    Connected = DsVeosCoSim_ConnectionState_Connected,
    Disconnected = DsVeosCoSim_ConnectionState_Disconnected
};

[[nodiscard]] inline std::string ToString(ConnectionState connectionState) {
    switch (connectionState) {
        case ConnectionState::Connected:
            return "Connected";
        case ConnectionState::Disconnected:
            return "Disconnected";
    }

    return std::to_string(static_cast<int>(connectionState));
}

[[nodiscard]] inline std::string ToString(DsVeosCoSim_DataType dataType) {
    switch (dataType) {
        case DsVeosCoSim_DataType_Bool:
            return "Bool";
        case DsVeosCoSim_DataType_Int8:
            return "Int8";
        case DsVeosCoSim_DataType_Int16:
            return "Int16";
        case DsVeosCoSim_DataType_Int32:
            return "Int32";
        case DsVeosCoSim_DataType_Int64:
            return "Int64";
        case DsVeosCoSim_DataType_UInt8:
            return "UInt8";
        case DsVeosCoSim_DataType_UInt16:
            return "UInt16";
        case DsVeosCoSim_DataType_UInt32:
            return "UInt32";
        case DsVeosCoSim_DataType_UInt64:
            return "UInt64";
        case DsVeosCoSim_DataType_Float32:
            return "Float32";
        case DsVeosCoSim_DataType_Float64:
            return "Float64";
        case DsVeosCoSim_DataType_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
    }

    return std::to_string(static_cast<int>(dataType));
}

[[nodiscard]] inline int GetDataTypeSize(DsVeosCoSim_DataType dataType) {
    switch (dataType) {
        case DsVeosCoSim_DataType_Bool:
        case DsVeosCoSim_DataType_Int8:
        case DsVeosCoSim_DataType_UInt8:
            return 1;
        case DsVeosCoSim_DataType_Int16:
        case DsVeosCoSim_DataType_UInt16:
            return 2;
        case DsVeosCoSim_DataType_Int32:
        case DsVeosCoSim_DataType_UInt32:
        case DsVeosCoSim_DataType_Float32:
            return 4;
        case DsVeosCoSim_DataType_Int64:
        case DsVeosCoSim_DataType_UInt64:
        case DsVeosCoSim_DataType_Float64:
            return 8;
        case DsVeosCoSim_DataType_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
    }

    return 0;
}

[[nodiscard]] inline std::string ToString(DsVeosCoSim_SizeKind sizeKind) {
    switch (sizeKind) {
        case DsVeosCoSim_SizeKind_Fixed:
            return "Fixed";
        case DsVeosCoSim_SizeKind_Variable:
            return "Variable";
        case DsVeosCoSim_SizeKind_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
    }

    return std::to_string(static_cast<int>(sizeKind));
}

enum class SimulationState {
};

enum class Mode {
};

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
        flagsStr += ",Loopback";
    }

    if (hasFlag(flags, CanMessageFlags::Error)) {
        flagsStr += ",Error";
    }

    if (hasFlag(flags, CanMessageFlags::Drop)) {
        flagsStr += ",Drop";
    }

    if (hasFlag(flags, CanMessageFlags::ExtendedId)) {
        flagsStr += ",ExtendedId";
    }

    if (hasFlag(flags, CanMessageFlags::BitRateSwitch)) {
        flagsStr += ",BitRateSwitch";
    }

    if (hasFlag(flags, CanMessageFlags::FlexibleDataRateFormat)) {
        flagsStr += ",FlexibleDataRateFormat";
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
        flagsStr += ",Loopback";
    }

    if (hasFlag(flags, EthMessageFlags::Error)) {
        flagsStr += ",Error";
    }

    if (hasFlag(flags, EthMessageFlags::Drop)) {
        flagsStr += ",Drop";
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
        flagsStr += ",Loopback";
    }

    if (hasFlag(flags, LinMessageFlags::Error)) {
        flagsStr += ",Error";
    }

    if (hasFlag(flags, LinMessageFlags::Drop)) {
        flagsStr += ",Drop";
    }

    if (hasFlag(flags, LinMessageFlags::Header)) {
        flagsStr += ",Header";
    }

    if (hasFlag(flags, LinMessageFlags::Response)) {
        flagsStr += ",Response";
    }

    if (hasFlag(flags, LinMessageFlags::WakeEvent)) {
        flagsStr += ",WakeEvent";
    }

    if (hasFlag(flags, LinMessageFlags::SleepEvent)) {
        flagsStr += ",SleepEvent";
    }

    if (hasFlag(flags, LinMessageFlags::EnhancedChecksum)) {
        flagsStr += ",EnhancedChecksum";
    }

    if (hasFlag(flags, LinMessageFlags::TransferOnce)) {
        flagsStr += ",TransferOnce";
    }

    if (hasFlag(flags, LinMessageFlags::ParityFailure)) {
        flagsStr += ",ParityFailure";
    }

    if (hasFlag(flags, LinMessageFlags::Collision)) {
        flagsStr += ",Collision";
    }

    if (hasFlag(flags, LinMessageFlags::NoResponse)) {
        flagsStr += ",NoResponse";
    }

    if (!flagsStr.empty()) {
        flagsStr.erase(0, 1);
    }

    return flagsStr;
}

using IoSignalId = DsVeosCoSim_IoSignalId;

struct IoSignal {
    IoSignalId id{};
    uint32_t length{};
    DsVeosCoSim_DataType dataType{};
    DsVeosCoSim_SizeKind sizeKind{};
    std::string name;

    [[nodiscard]] DsVeosCoSim_IoSignal Convert() const {
        DsVeosCoSim_IoSignal signal{};
        signal.id = id;
        signal.length = length;
        signal.dataType = dataType;
        signal.sizeKind = sizeKind;
        signal.name = name.c_str();
        return signal;
    }
};

[[nodiscard]] inline std::vector<DsVeosCoSim_IoSignal> Convert(const std::vector<IoSignal>& signals) {
    std::vector<DsVeosCoSim_IoSignal> ioSignals;
    ioSignals.reserve(signals.size());

    for (const auto& signal : signals) {
        ioSignals.push_back(signal.Convert());
    }

    return ioSignals;
}

using BusControllerId = DsVeosCoSim_BusControllerId;

struct CanController {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    uint64_t flexibleDataRateBitsPerSecond{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] DsVeosCoSim_CanController Convert() const {
        DsVeosCoSim_CanController controller{};
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

[[nodiscard]] inline std::vector<DsVeosCoSim_CanController> Convert(const std::vector<CanController>& controllers) {
    std::vector<DsVeosCoSim_CanController> canControllers;
    canControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        canControllers.push_back(controller.Convert());
    }

    return canControllers;
}

struct EthController {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    std::array<uint8_t, EthAddressLength> macAddress{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] DsVeosCoSim_EthController Convert() const {
        DsVeosCoSim_EthController controller{};
        controller.id = id;
        controller.queueSize = queueSize;
        controller.bitsPerSecond = bitsPerSecond;
        (void)std::memcpy(controller.macAddress, macAddress.data(), EthAddressLength);
        controller.name = name.c_str();
        controller.channelName = channelName.c_str();
        controller.clusterName = clusterName.c_str();
        return controller;
    }
};

[[nodiscard]] inline std::vector<DsVeosCoSim_EthController> Convert(const std::vector<EthController>& controllers) {
    std::vector<DsVeosCoSim_EthController> ethControllers;
    ethControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        ethControllers.push_back(controller.Convert());
    }

    return ethControllers;
}

enum class LinControllerType {
    Responder = DsVeosCoSim_LinControllerType_Responder,
    Commander = DsVeosCoSim_LinControllerType_Commander
};

[[nodiscard]] inline std::string ToString(LinControllerType type) {
    switch (type) {
        case LinControllerType::Responder:
            return "Responder";
        case LinControllerType::Commander:
            return "Commander";
    }

    return std::to_string(static_cast<int>(type));
}

struct LinController {
    BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    LinControllerType type{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] DsVeosCoSim_LinController Convert() const {
        DsVeosCoSim_LinController controller{};
        controller.id = id;
        controller.queueSize = queueSize;
        controller.bitsPerSecond = bitsPerSecond;
        controller.type = (DsVeosCoSim_LinControllerType)type;
        controller.name = name.c_str();
        controller.channelName = channelName.c_str();
        controller.clusterName = clusterName.c_str();
        return controller;
    }
};

[[nodiscard]] inline std::vector<DsVeosCoSim_LinController> Convert(const std::vector<LinController>& controllers) {
    std::vector<DsVeosCoSim_LinController> linControllers;
    linControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        linControllers.push_back(controller.Convert());
    }

    return linControllers;
}

using LogCallback = std::function<void(Severity, std::string_view)>;

using SimulationCallback = std::function<void(SimulationTime simulationTime)>;
using SimulationTerminatedCallback = std::function<void(SimulationTime simulationTime, TerminateReason reason)>;
using IncomingSignalChangedCallback =
    std::function<void(SimulationTime simulationTime, const DsVeosCoSim_IoSignal& ioSignal, uint32_t length, const void* value)>;
using CanMessageReceivedCallback =
    std::function<void(SimulationTime simulationTime, const DsVeosCoSim_CanController& controller, const DsVeosCoSim_CanMessage& message)>;
using EthMessageReceivedCallback =
    std::function<void(SimulationTime simulationTime, const DsVeosCoSim_EthController& controller, const DsVeosCoSim_EthMessage& message)>;
using LinMessageReceivedCallback =
    std::function<void(SimulationTime simulationTime, const DsVeosCoSim_LinController& controller, const DsVeosCoSim_LinMessage& message)>;

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

struct ConnectConfig {
    std::string remoteIpAddress;
    std::string serverName;
    std::string clientName;
    uint16_t remotePort{};
    uint16_t localPort{};
};

}  // namespace DsVeosCoSim
