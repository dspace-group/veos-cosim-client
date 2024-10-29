// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <memory.h>

#include <array>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

#include "DsVeosCoSim/DsVeosCoSim.h"

namespace DsVeosCoSim {

constexpr uint32_t CanMessageMaxLength = DSVEOSCOSIM_CAN_MESSAGE_MAX_LENGTH;  // NOLINT
constexpr uint32_t EthMessageMaxLength = DSVEOSCOSIM_ETH_MESSAGE_MAX_LENGTH;  // NOLINT
constexpr uint32_t LinMessageMaxLength = DSVEOSCOSIM_LIN_MESSAGE_MAX_LENGTH;  // NOLINT
constexpr uint32_t EthAddressLength = DSVEOSCOSIM_ETH_ADDRESS_LENGTH;

[[nodiscard]] inline std::string SimulationTimeToString(DsVeosCoSim_SimulationTime simulationTime) {
    return std::to_string(DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime));
}

[[nodiscard]] inline std::string ToString(DsVeosCoSim_Result result) {
    switch (result) {
        case DsVeosCoSim_Result_Ok:
            return "Ok";
        case DsVeosCoSim_Result_Error:
            return "Error";
        case DsVeosCoSim_Result_Empty:
            return "Empty";
        case DsVeosCoSim_Result_Full:
            return "Full";
        case DsVeosCoSim_Result_InvalidArgument:
            return "InvalidArgument";
        case DsVeosCoSim_Result_Disconnected:
            return "Disconnected";
        case DsVeosCoSim_Result_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
    }

    return "<Invalid DsVeosCoSim_Result>";
}

enum class CoSimType {
    Client,
    Server
};

[[nodiscard]] inline std::string ToString(CoSimType coSimType) {
    switch (coSimType) {
        case CoSimType::Client:
            return "Client";
        case CoSimType::Server:
            return "Server";
    }

    return "<Invalid CoSimType>";
}

enum class ConnectionKind {
    Remote,
    Local
};

[[nodiscard]] inline std::string ToString(ConnectionKind connectionKind) {
    switch (connectionKind) {
        case ConnectionKind::Remote:
            return "Remote";
        case ConnectionKind::Local:
            return "Local";
    }

    return "<Invalid ConnectionKind>";
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

    return "<Invalid Command>";
}

[[nodiscard]] inline std::string ToString(DsVeosCoSim_Severity severity) {
    switch (severity) {
        case DsVeosCoSim_Severity_Error:
            return "Error";
        case DsVeosCoSim_Severity_Warning:
            return "Warning";
        case DsVeosCoSim_Severity_Info:
            return "Info";
        case DsVeosCoSim_Severity_Trace:
            return "Trace";
        case DsVeosCoSim_Severity_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
    }

    return "<Invalid DsVeosCoSim_Severity>";
}

[[nodiscard]] inline std::string ToString(DsVeosCoSim_TerminateReason terminateReason) {
    switch (terminateReason) {
        case DsVeosCoSim_TerminateReason_Finished:
            return "Finished";
        case DsVeosCoSim_TerminateReason_Error:
            return "Error";
        case DsVeosCoSim_TerminateReason_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
    }

    return "<Invalid DsVeosCoSim_TerminateReason>";
}

[[nodiscard]] inline std::string ToString(DsVeosCoSim_ConnectionState connectionState) {
    switch (connectionState) {
        case DsVeosCoSim_ConnectionState_Connected:
            return "Connected";
        case DsVeosCoSim_ConnectionState_Disconnected:
            return "Disconnected";
        case DsVeosCoSim_ConnectionState_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
    }

    return "<Invalid DsVeosCoSim_ConnectionState>";
}

[[nodiscard]] inline size_t GetDataTypeSize(DsVeosCoSim_DataType dataType) {
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

    return "<Invalid DsVeosCoSim_DataType>";
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

    return "<Invalid DsVeosCoSim_SizeKind>";
}

[[nodiscard]] inline std::string DataTypeValueToString(DsVeosCoSim_DataType dataType,
                                                       uint32_t index,
                                                       const void* value) {
    switch (dataType) {
        case DsVeosCoSim_DataType_Bool:
            return std::to_string(static_cast<const uint8_t*>(value)[index]);
        case DsVeosCoSim_DataType_Int8:
            return std::to_string(static_cast<const int8_t*>(value)[index]);
        case DsVeosCoSim_DataType_Int16:
            return std::to_string(static_cast<const int16_t*>(value)[index]);
        case DsVeosCoSim_DataType_Int32:
            return std::to_string(static_cast<const int32_t*>(value)[index]);
        case DsVeosCoSim_DataType_Int64:
            return std::to_string(static_cast<const int64_t*>(value)[index]);
        case DsVeosCoSim_DataType_UInt8:
            return std::to_string(static_cast<const uint8_t*>(value)[index]);
        case DsVeosCoSim_DataType_UInt16:
            return std::to_string(static_cast<const uint16_t*>(value)[index]);
        case DsVeosCoSim_DataType_UInt32:
            return std::to_string(static_cast<const uint32_t*>(value)[index]);
        case DsVeosCoSim_DataType_UInt64:
            return std::to_string(static_cast<const uint64_t*>(value)[index]);
        case DsVeosCoSim_DataType_Float32:
            return std::to_string(static_cast<const float*>(value)[index]);
        case DsVeosCoSim_DataType_Float64:
            return std::to_string(static_cast<const double*>(value)[index]);
        case DsVeosCoSim_DataType_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
    }

    throw std::runtime_error("Invalid data type.");
}

[[nodiscard]] inline std::string ValueToString(DsVeosCoSim_DataType dataType, uint32_t length, const void* value) {
    std::ostringstream oss;
    for (uint32_t i = 0; i < length; i++) {
        if (i > 0) {
            oss << " ";
        }

        oss << DataTypeValueToString(dataType, i, value);
    }

    return oss.str();
}

[[nodiscard]] inline std::string ToString(DsVeosCoSim_LinControllerType type) {
    switch (type) {
        case DsVeosCoSim_LinControllerType_Responder:
            return "Responder";
        case DsVeosCoSim_LinControllerType_Commander:
            return "Commander";
        case DsVeosCoSim_LinControllerType_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
    }

    return "<Invalid DsVeosCoSim_LinControllerType>";
}

enum class SimulationState {
    Unloaded,
    Stopped,
    Running,
    Paused,
    Terminated
};

[[nodiscard]] inline std::string ToString([[maybe_unused]] SimulationState simulationState) {
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

[[nodiscard]] inline std::string ToString([[maybe_unused]] Mode mode) {
    return "<Unused>";
}

[[nodiscard]] inline std::string CanMessageFlagsToString(DsVeosCoSim_CanMessageFlags flags) {
    std::string flagsStr;

    if ((flags & DsVeosCoSim_CanMessageFlags_Loopback) != 0) {
        flagsStr += ",Loopback";
    }

    if ((flags & DsVeosCoSim_CanMessageFlags_Error) != 0) {
        flagsStr += ",Error";
    }

    if ((flags & DsVeosCoSim_CanMessageFlags_Drop) != 0) {
        flagsStr += ",Drop";
    }

    if ((flags & DsVeosCoSim_CanMessageFlags_ExtendedId) != 0) {
        flagsStr += ",ExtendedId";
    }

    if ((flags & DsVeosCoSim_CanMessageFlags_BitRateSwitch) != 0) {
        flagsStr += ",BitRateSwitch";
    }

    if ((flags & DsVeosCoSim_CanMessageFlags_FlexibleDataRateFormat) != 0) {
        flagsStr += ",FlexibleDataRateFormat";
    }

    if (!flagsStr.empty()) {
        flagsStr.erase(0, 1);
    }

    return flagsStr;
}

[[nodiscard]] inline std::string EthMessageFlagsToString(DsVeosCoSim_EthMessageFlags flags) {
    std::string flagsStr;

    if ((flags & DsVeosCoSim_EthMessageFlags_Loopback) != 0) {
        flagsStr += ",Loopback";
    }

    if ((flags & DsVeosCoSim_EthMessageFlags_Error) != 0) {
        flagsStr += ",Error";
    }

    if ((flags & DsVeosCoSim_EthMessageFlags_Drop) != 0) {
        flagsStr += ",Drop";
    }

    if (!flagsStr.empty()) {
        flagsStr.erase(0, 1);
    }

    return flagsStr;
}

[[nodiscard]] inline std::string LinMessageFlagsToString(DsVeosCoSim_LinMessageFlags flags) {
    std::string flagsStr;

    if ((flags & DsVeosCoSim_LinMessageFlags_Loopback) != 0) {
        flagsStr += ",Loopback";
    }

    if ((flags & DsVeosCoSim_LinMessageFlags_Error) != 0) {
        flagsStr += ",Error";
    }

    if ((flags & DsVeosCoSim_LinMessageFlags_Drop) != 0) {
        flagsStr += ",Drop";
    }

    if ((flags & DsVeosCoSim_LinMessageFlags_Header) != 0) {
        flagsStr += ",Header";
    }

    if ((flags & DsVeosCoSim_LinMessageFlags_Response) != 0) {
        flagsStr += ",Response";
    }

    if ((flags & DsVeosCoSim_LinMessageFlags_WakeEvent) != 0) {
        flagsStr += ",WakeEvent";
    }

    if ((flags & DsVeosCoSim_LinMessageFlags_SleepEvent) != 0) {
        flagsStr += ",SleepEvent";
    }

    if ((flags & DsVeosCoSim_LinMessageFlags_EnhancedChecksum) != 0) {
        flagsStr += ",EnhancedChecksum";
    }

    if ((flags & DsVeosCoSim_LinMessageFlags_TransferOnce) != 0) {
        flagsStr += ",TransferOnce";
    }

    if ((flags & DsVeosCoSim_LinMessageFlags_ParityFailure) != 0) {
        flagsStr += ",ParityFailure";
    }

    if ((flags & DsVeosCoSim_LinMessageFlags_Collision) != 0) {
        flagsStr += ",Collision";
    }

    if ((flags & DsVeosCoSim_LinMessageFlags_NoResponse) != 0) {
        flagsStr += ",NoResponse";
    }

    if (!flagsStr.empty()) {
        flagsStr.erase(0, 1);
    }

    return flagsStr;
}

[[nodiscard]] inline std::string DataToString(const uint8_t* data, size_t dataLength, char separator = 0) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (uint32_t i = 0; i < dataLength; i++) {
        oss << std::setw(2) << static_cast<int32_t>(data[i]);
        if ((i < dataLength - 1) && separator != 0) {
            oss << separator;
        }
    }

    return oss.str();
}

[[nodiscard]] inline std::string IoDataToString(DsVeosCoSim_SimulationTime simulationTime,
                                                const DsVeosCoSim_IoSignal& ioSignal,
                                                uint32_t length,
                                                const void* value) {
    return DsVeosCoSim_SimulationTimeToString(simulationTime) + "," + ioSignal.name + "," +
           std::to_string(length) + "," + DsVeosCoSim_ValueToString(ioSignal.dataType, length, value);
}

struct IoSignal {
    DsVeosCoSim_IoSignalId id{};
    uint32_t length{};
    DsVeosCoSim_DataType dataType{};
    DsVeosCoSim_SizeKind sizeKind{};
    std::string name;

    [[nodiscard]] operator DsVeosCoSim_IoSignal() const {  // NOLINT
        DsVeosCoSim_IoSignal signal{};
        signal.id = id;
        signal.length = length;
        signal.dataType = dataType;
        signal.sizeKind = sizeKind;
        signal.name = name.c_str();
        return signal;
    }
};

[[nodiscard]] inline std::string ToString(const DsVeosCoSim_IoSignal& signal) {
    std::string str = "{Id: " + std::to_string(signal.id) + ", Length: " + std::to_string(signal.length) +
                      ", DataType: " + ToString(signal.dataType) + ", SizeKind: " + ToString(signal.sizeKind) +
                      ", Name: \"" + signal.name + "\"}";

    return str;
}

[[nodiscard]] inline std::string ToString(const std::vector<IoSignal>& signals) {
    std::string str = "[";

    bool first = true;
    for (const IoSignal& signal : signals) {
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

[[nodiscard]] inline std::vector<DsVeosCoSim_IoSignal> Convert(const std::vector<IoSignal>& signals) {
    std::vector<DsVeosCoSim_IoSignal> ioSignals;
    ioSignals.reserve(signals.size());

    for (const auto& signal : signals) {
        ioSignals.push_back(signal);
    }

    return ioSignals;
}

[[nodiscard]] inline std::string CanMessageToString(DsVeosCoSim_SimulationTime simulationTime,
                                                    const DsVeosCoSim_CanController& controller,
                                                    const DsVeosCoSim_CanMessage& message) {
    return DsVeosCoSim_SimulationTimeToString(simulationTime) + "," + controller.name + "," +
           std::to_string(message.id) + "," + std::to_string(message.length) + "," +
           DataToString(message.data, message.length, '-') + ",CAN," + CanMessageFlagsToString(message.flags);
}

struct CanController {
    DsVeosCoSim_BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    uint64_t flexibleDataRateBitsPerSecond{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] operator DsVeosCoSim_CanController() const {  // NOLINT
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

[[nodiscard]] inline std::string ToString(const DsVeosCoSim_CanController& controller) {
    std::string str = "{Id: " + std::to_string(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
                      ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) +
                      ", FlexibleDataRateBitsPerSecond: " + std::to_string(controller.flexibleDataRateBitsPerSecond) +
                      ", Name: \"" + controller.name + "\", ChannelName: \"" + controller.channelName +
                      "\", ClusterName: \"" + controller.clusterName + "\"}";

    return str;
}

[[nodiscard]] inline std::string ToString(const std::vector<CanController>& controllers) {
    std::string str = "[";

    bool first = true;
    for (const CanController& controller : controllers) {
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

[[nodiscard]] inline std::vector<DsVeosCoSim_CanController> Convert(const std::vector<CanController>& controllers) {
    std::vector<DsVeosCoSim_CanController> canControllers;
    canControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        canControllers.push_back(controller);
    }

    return canControllers;
}

[[nodiscard]] inline std::string EthMessageToString(DsVeosCoSim_SimulationTime simulationTime,
                                                    const DsVeosCoSim_EthController& controller,
                                                    const DsVeosCoSim_EthMessage& message) {
    const uint8_t* data = message.data;
    const uint32_t length = message.length;

    if (length >= 14) {
        const std::string macAddress1 = DataToString(message.data, 6, ':');
        const std::string macAddress2 = DataToString(message.data + 6, 6, ':');
        const std::string ethernetType = DataToString(message.data + 12, 2);

        return DsVeosCoSim_SimulationTimeToString(simulationTime) + "," + controller.name + "," +
               macAddress2 + "-" + macAddress1 + "," + std::to_string(length - 14) + "," +
               DataToString(data + 14, length - 14, '-') + ",ETH," + ethernetType + "," +
               EthMessageFlagsToString(message.flags);
    }

    return DsVeosCoSim_SimulationTimeToString(simulationTime) + "," + controller.name + "," +
           std::to_string(length) + "," + DataToString(data, length, '-') + ",ETH," +
           EthMessageFlagsToString(message.flags);
}

struct EthController {
    DsVeosCoSim_BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    std::array<uint8_t, EthAddressLength> macAddress{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] operator DsVeosCoSim_EthController() const {  // NOLINT
        DsVeosCoSim_EthController controller{};
        controller.id = id;
        controller.queueSize = queueSize;
        controller.bitsPerSecond = bitsPerSecond;
        (void)::memcpy(controller.macAddress, macAddress.data(), EthAddressLength);
        controller.name = name.c_str();
        controller.channelName = channelName.c_str();
        controller.clusterName = clusterName.c_str();
        return controller;
    }
};

[[nodiscard]] inline std::string ToString(const DsVeosCoSim_EthController& controller) {
    std::string str = "{Id: " + std::to_string(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
                      ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) + ", MacAddress: [" +
                      DataToString(controller.macAddress, sizeof(controller.macAddress), ':') + "], Name: \"" +
                      controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
                      controller.clusterName + "\"}";

    return str;
}

[[nodiscard]] inline std::string ToString(const std::vector<EthController>& controllers) {
    std::string str = "[";

    bool first = true;
    for (const EthController& controller : controllers) {
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

[[nodiscard]] inline std::vector<DsVeosCoSim_EthController> Convert(const std::vector<EthController>& controllers) {
    std::vector<DsVeosCoSim_EthController> ethControllers;
    ethControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        ethControllers.push_back(controller);
    }

    return ethControllers;
}

[[nodiscard]] inline std::string LinMessageToString(DsVeosCoSim_SimulationTime simulationTime,
                                                    const DsVeosCoSim_LinController& controller,
                                                    const DsVeosCoSim_LinMessage& message) {
    return DsVeosCoSim_SimulationTimeToString(simulationTime) + "," + controller.name + "," +
           std::to_string(message.id) + "," + std::to_string(message.length) + "," +
           DataToString(message.data, message.length, '-') + ",LIN," + LinMessageFlagsToString(message.flags);
}

struct LinController {
    DsVeosCoSim_BusControllerId id{};
    uint32_t queueSize{};
    uint64_t bitsPerSecond{};
    DsVeosCoSim_LinControllerType type{};
    std::string name;
    std::string channelName;
    std::string clusterName;

    [[nodiscard]] operator DsVeosCoSim_LinController() const {  // NOLINT
        DsVeosCoSim_LinController controller{};
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

[[nodiscard]] inline std::string ToString(const DsVeosCoSim_LinController& controller) {
    std::string str = "{Id: " + std::to_string(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
                      ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) +
                      ", Type: " + ToString(controller.type) + ", Name: \"" + controller.name + "\", ChannelName: \"" +
                      controller.channelName + "\", ClusterName: \"" + controller.clusterName + "\"}";

    return str;
}

[[nodiscard]] inline std::string ToString(const std::vector<LinController>& controllers) {
    std::string str = "[";

    bool first = true;
    for (const LinController& controller : controllers) {
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

[[nodiscard]] inline std::vector<DsVeosCoSim_LinController> Convert(const std::vector<LinController>& controllers) {
    std::vector<DsVeosCoSim_LinController> linControllers;
    linControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        linControllers.push_back(controller);
    }

    return linControllers;
}

using LogCallback = std::function<void(DsVeosCoSim_Severity, std::string_view)>;

using SimulationCallback = std::function<void(DsVeosCoSim_SimulationTime simulationTime)>;
using SimulationTerminatedCallback =
    std::function<void(DsVeosCoSim_SimulationTime simulationTime, DsVeosCoSim_TerminateReason reason)>;
using IncomingSignalChangedCallback = std::function<void(DsVeosCoSim_SimulationTime simulationTime,
                                                         const DsVeosCoSim_IoSignal& ioSignal,
                                                         uint32_t length,
                                                         const void* value)>;
using CanMessageReceivedCallback = std::function<void(DsVeosCoSim_SimulationTime simulationTime,
                                                      const DsVeosCoSim_CanController& controller,
                                                      const DsVeosCoSim_CanMessage& message)>;
using EthMessageReceivedCallback = std::function<void(DsVeosCoSim_SimulationTime simulationTime,
                                                      const DsVeosCoSim_EthController& controller,
                                                      const DsVeosCoSim_EthMessage& message)>;
using LinMessageReceivedCallback = std::function<void(DsVeosCoSim_SimulationTime simulationTime,
                                                      const DsVeosCoSim_LinController& controller,
                                                      const DsVeosCoSim_LinMessage& message)>;

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
