// Copyright dSPACE GmbH. All rights reserved.

#include "DsVeosCoSim/CoSimTypes.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace DsVeosCoSim {

namespace {

[[nodiscard]] std::string DataTypeValueToString(const DataType dataType, const uint32_t index, const void* value) {
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

}  // namespace

[[nodiscard]] std::string DataToString(const uint8_t* data, const size_t dataLength, const char separator) {
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

[[nodiscard]] std::string SimulationTimeToString(const SimulationTime simulationTime) {
    const int64_t nanoseconds = simulationTime.count();
    std::string str = std::to_string(nanoseconds);

    const size_t length = str.size();
    if (length < 10) {
        const size_t countOfMissingZerosInFront = 10 - length;
        str.insert(str.begin(), countOfMissingZerosInFront, '0');
    }

    str.insert(str.size() - 9, ".");
    while (str[str.size() - 1] == '0') {
        str.pop_back();
    }

    if (str[str.size() - 1] == '.') {
        str.pop_back();
    }

    return str;
}

[[nodiscard]] std::string_view ToString(const Result result) {
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

[[nodiscard]] std::string_view ToString(const CoSimType coSimType) {
    switch (coSimType) {
        case CoSimType::Client:
            return "Client";
        case CoSimType::Server:
            return "Server";
    }

    return "<Invalid CoSimType>";
}

[[nodiscard]] std::string_view ToString(const ConnectionKind connectionKind) {
    switch (connectionKind) {
        case ConnectionKind::Remote:
            return "Remote";
        case ConnectionKind::Local:
            return "Local";
    }

    return "<Invalid ConnectionKind>";
}

[[nodiscard]] std::string_view ToString(const Command command) {
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

[[nodiscard]] std::string_view ToString(const Severity severity) {
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

[[nodiscard]] std::string_view ToString(const TerminateReason terminateReason) {
    switch (terminateReason) {
        case TerminateReason::Finished:
            return "Finished";
        case TerminateReason::Error:
            return "Error";
    }

    return "<Invalid TerminateReason>";
}

[[nodiscard]] std::string_view ToString(const ConnectionState connectionState) {
    switch (connectionState) {
        case ConnectionState::Disconnected:
            return "Disconnected";
        case ConnectionState::Connected:
            return "Connected";
    }

    return "<Invalid ConnectionState>";
}

[[nodiscard]] size_t GetDataTypeSize(const DataType dataType) {
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

[[nodiscard]] std::string_view ToString(const DataType dataType) {
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

[[nodiscard]] std::string_view ToString(const SizeKind sizeKind) {
    switch (sizeKind) {
        case SizeKind::Fixed:
            return "Fixed";
        case SizeKind::Variable:
            return "Variable";
    }

    return "<Invalid SizeKind>";
}

[[nodiscard]] std::string ValueToString(const DataType dataType, const uint32_t length, const void* value) {
    std::ostringstream oss;
    for (uint32_t i = 0; i < length; i++) {
        if (i > 0) {
            oss << " ";
        }

        oss << DataTypeValueToString(dataType, i, value);
    }

    return oss.str();
}

[[nodiscard]] std::string_view ToString(const SimulationState simulationState) {
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

[[nodiscard]] std::string_view ToString([[maybe_unused]] const Mode mode) {
    return "<Unused>";
}

[[nodiscard]] std::string ToString(const IoSignalId signalId) {
    return std::to_string(static_cast<uint32_t>(signalId));
}

[[nodiscard]] IoSignalContainer::operator IoSignal() const {
    IoSignal signal{};
    signal.id = id;
    signal.length = length;
    signal.dataType = dataType;
    signal.sizeKind = sizeKind;
    signal.name = name.c_str();
    return signal;
}

[[nodiscard]] std::string IoDataToString(const IoSignal& ioSignal, const uint32_t length, const void* value) {
    std::string str = "IO Data { Id: ";
    str.append(ToString(ioSignal.id));
    str.append(", Length: ");
    str.append(std::to_string(length));
    str.append(", Data: ");
    str.append(ValueToString(ioSignal.dataType, length, value));
    str.append(" }");
    return str;
}

[[nodiscard]] std::string ToString(const IoSignal& signal) {
    std::string str = "IO Signal { Id: ";
    str.append(ToString(signal.id));
    str.append(", Length: ");
    str.append(std::to_string(signal.length));
    str.append(", DataType: ");
    str.append(ToString(signal.dataType));
    str.append(", SizeKind: ");
    str.append(ToString(signal.sizeKind));
    str.append(", Name: \"");
    str.append(signal.name);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string ToString(const IoSignalContainer& signal) {
    std::string str = "IO Signal { Id: ";
    str.append(ToString(signal.id));
    str.append(", Length: ");
    str.append(std::to_string(signal.length));
    str.append(", DataType: ");
    str.append(ToString(signal.dataType));
    str.append(", SizeKind: ");
    str.append(ToString(signal.sizeKind));
    str.append(", Name: \"");
    str.append(signal.name);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string ToString(const std::vector<IoSignalContainer>& signals) {
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

[[nodiscard]] std::vector<IoSignal> Convert(const std::vector<IoSignalContainer>& signals) {
    std::vector<IoSignal> ioSignals;
    ioSignals.reserve(signals.size());

    for (const auto& signal : signals) {
        ioSignals.push_back(static_cast<IoSignal>(signal));
    }

    return ioSignals;
}

[[nodiscard]] std::string ToString(const BusControllerId busControllerId) {
    return std::to_string(static_cast<uint32_t>(busControllerId));
}

[[nodiscard]] std::string ToString(const BusMessageId busMessageId) {
    return std::to_string(static_cast<uint32_t>(busMessageId));
}

[[nodiscard]] std::string ToString(const CanMessageFlags flags) {
    std::string str;

    if (HasFlag(flags, CanMessageFlags::Loopback)) {
        str.append(",Loopback");
    }

    if (HasFlag(flags, CanMessageFlags::Error)) {
        str.append(",Error");
    }

    if (HasFlag(flags, CanMessageFlags::Drop)) {
        str.append(",Drop");
    }

    if (HasFlag(flags, CanMessageFlags::ExtendedId)) {
        str.append(",ExtendedId");
    }

    if (HasFlag(flags, CanMessageFlags::BitRateSwitch)) {
        str.append(",BitRateSwitch");
    }

    if (HasFlag(flags, CanMessageFlags::FlexibleDataRateFormat)) {
        str.append(",FlexibleDataRateFormat");
    }

    if (!str.empty()) {
        str.erase(0, 1);
    }

    return str;
}

[[nodiscard]] CanControllerContainer::operator CanController() const {
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

[[nodiscard]] CanMessage::operator CanMessageContainer() const {
    CanMessageContainer message{};
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = flags;
    message.length = length;
    (void)memcpy(message.data.data(), data, length);
    return message;
}

[[nodiscard]] CanMessageContainer::operator CanMessage() const {
    CheckMaxLength();
    CheckFlags();
    CanMessage message{};
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = flags;
    message.length = length;
    message.data = data.data();
    return message;
}

void CanMessageContainer::CheckMaxLength() const {
    if (length > CanMessageMaxLength) {
        throw std::runtime_error("CAN message data exceeds maximum length.");
    }
}

void CanMessageContainer::CheckFlags() const {
    if (!HasFlag(flags, CanMessageFlags::FlexibleDataRateFormat)) {
        if (length > 8) {
            throw std::runtime_error("CAN message flags invalid. A DLC > 8 requires the flexible data rate format flag.");
        }

        if (HasFlag(flags, CanMessageFlags::BitRateSwitch)) {
            throw std::runtime_error(
                "CAN message flags invalid. A bit rate switch flag requires the flexible data rate format flag.");
        }
    }
}

[[nodiscard]] std::string ToString(const CanMessage& message) {
    std::string str = "CAN Message { Timestamp: ";
    str.append(SimulationTimeToString(message.timestamp));
    str.append(", ControllerId: ");
    str.append(ToString(message.controllerId));
    str.append(", Id: ");
    str.append(ToString(message.id));
    str.append(", Length: ");
    str.append(std::to_string(message.length));
    str.append(", Data: ");
    str.append(DataToString(message.data, message.length, '-'));
    str.append(", Flags: ");
    str.append(ToString(message.flags));
    str.append(" }");
    return str;
}

[[nodiscard]] std::string ToString(const CanMessageContainer& message) {
    std::string str = "CAN Message { Timestamp: ";
    str.append(SimulationTimeToString(message.timestamp));
    str.append(", ControllerId: ");
    str.append(ToString(message.controllerId));
    str.append(", Id: ");
    str.append(ToString(message.id));
    str.append(", Length: ");
    str.append(std::to_string(message.length));
    str.append(", Data: ");
    str.append(DataToString(message.data.data(), message.length, '-'));
    str.append(", Flags: ");
    str.append(ToString(message.flags));
    str.append(" }");
    return str;
}

[[nodiscard]] std::string ToString(const CanController& controller) {
    std::string str = "CAN Controller { Id: ";
    str.append(ToString(controller.id));
    str.append(", QueueSize: ");
    str.append(std::to_string(controller.queueSize));
    str.append(", BitsPerSecond: ");
    str.append(std::to_string(controller.bitsPerSecond));
    str.append(", FlexibleDataRateBitsPerSecond: ");
    str.append(std::to_string(controller.flexibleDataRateBitsPerSecond));
    str.append(", Name: \"");
    str.append(controller.name);
    str.append("\", ChannelName: \"");
    str.append(controller.channelName);
    str.append("\", ClusterName: \"");
    str.append(controller.clusterName);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string ToString(const CanControllerContainer& controller) {
    std::string str = "CAN Controller { Id: ";
    str.append(ToString(controller.id));
    str.append(", QueueSize: ");
    str.append(std::to_string(controller.queueSize));
    str.append(", BitsPerSecond: ");
    str.append(std::to_string(controller.bitsPerSecond));
    str.append(", FlexibleDataRateBitsPerSecond: ");
    str.append(std::to_string(controller.flexibleDataRateBitsPerSecond));
    str.append(", Name: \"");
    str.append(controller.name);
    str.append("\", ChannelName: \"");
    str.append(controller.channelName);
    str.append("\", ClusterName: \"");
    str.append(controller.clusterName);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string ToString(const std::vector<CanControllerContainer>& controllers) {
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

[[nodiscard]] std::vector<CanController> Convert(const std::vector<CanControllerContainer>& controllers) {
    std::vector<CanController> canControllers;
    canControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        canControllers.push_back(static_cast<CanController>(controller));
    }

    return canControllers;
}

[[nodiscard]] std::string ToString(const EthMessageFlags flags) {
    std::string str;

    if (HasFlag(flags, EthMessageFlags::Loopback)) {
        str.append(",Loopback");
    }

    if (HasFlag(flags, EthMessageFlags::Error)) {
        str.append(",Error");
    }

    if (HasFlag(flags, EthMessageFlags::Drop)) {
        str.append(",Drop");
    }

    if (!str.empty()) {
        str.erase(0, 1);
    }

    return str;
}

[[nodiscard]] EthControllerContainer::operator EthController() const {
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

[[nodiscard]] EthMessage::operator EthMessageContainer() const {
    EthMessageContainer message{};
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.flags = flags;
    message.length = length;
    (void)memcpy(message.data.data(), data, length);
    return message;
}

[[nodiscard]] EthMessageContainer::operator EthMessage() const {
    CheckMaxLength();
    EthMessage message{};
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.flags = flags;
    message.length = length;
    message.data = data.data();
    return message;
}

void EthMessageContainer::CheckMaxLength() const {
    if (length > EthMessageMaxLength) {
        throw std::runtime_error("Ethernet message data exceeds maximum length.");
    }
}

[[nodiscard]] std::string ToString(const EthMessage& message) {
    std::string str = "ETH Message { Timestamp: ";
    str.append(SimulationTimeToString(message.timestamp));
    str.append(", ControllerId: ");
    str.append(ToString(message.controllerId));
    str.append(", Length: ");
    str.append(std::to_string(message.length));
    str.append(", Data: ");
    str.append(DataToString(message.data, message.length, '-'));
    str.append(", Flags: ");
    str.append(ToString(message.flags));
    str.append(" }");
    return str;
}

[[nodiscard]] std::string ToString(const EthMessageContainer& message) {
    std::string str = "ETH Message { Timestamp: ";
    str.append(SimulationTimeToString(message.timestamp));
    str.append(", ControllerId: ");
    str.append(ToString(message.controllerId));
    str.append(", Length: ");
    str.append(std::to_string(message.length));
    str.append(", Data: ");
    str.append(DataToString(message.data.data(), message.length, '-'));
    str.append(", Flags: ");
    str.append(ToString(message.flags));
    str.append(" }");
    return str;
}

[[nodiscard]] std::string ToString(const EthController& controller) {
    std::string str = "ETH Controller { Id: ";
    str.append(ToString(controller.id));
    str.append(", QueueSize: ");
    str.append(std::to_string(controller.queueSize));
    str.append(", BitsPerSecond: ");
    str.append(std::to_string(controller.bitsPerSecond));
    str.append(", MacAddress: [");
    str.append(DataToString(controller.macAddress.data(), sizeof(controller.macAddress), ':'));
    str.append("], Name: \"");
    str.append(controller.name);
    str.append("\", ChannelName: \"");
    str.append(controller.channelName);
    str.append("\", ClusterName: \"");
    str.append(controller.clusterName);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string ToString(const EthControllerContainer& controller) {
    std::string str = "ETH Controller { Id: ";
    str.append(ToString(controller.id));
    str.append(", QueueSize: ");
    str.append(std::to_string(controller.queueSize));
    str.append(", BitsPerSecond: ");
    str.append(std::to_string(controller.bitsPerSecond));
    str.append(", MacAddress: [");
    str.append(DataToString(controller.macAddress.data(), sizeof(controller.macAddress), ':'));
    str.append("], Name: \"");
    str.append(controller.name);
    str.append("\", ChannelName: \"");
    str.append(controller.channelName);
    str.append("\", ClusterName: \"");
    str.append(controller.clusterName);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string ToString(const std::vector<EthControllerContainer>& controllers) {
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

[[nodiscard]] std::vector<EthController> Convert(const std::vector<EthControllerContainer>& controllers) {
    std::vector<EthController> ethControllers;
    ethControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        ethControllers.push_back(static_cast<EthController>(controller));
    }

    return ethControllers;
}

[[nodiscard]] std::string_view ToString(const LinControllerType type) {
    switch (type) {
        case LinControllerType::Responder:
            return "Responder";
        case LinControllerType::Commander:
            return "Commander";
    }

    return "<Invalid LinControllerType>";
}

[[nodiscard]] std::string ToString(const LinMessageFlags flags) {
    std::string str;

    if (HasFlag(flags, LinMessageFlags::Loopback)) {
        str.append(",Loopback");
    }

    if (HasFlag(flags, LinMessageFlags::Error)) {
        str.append(",Error");
    }

    if (HasFlag(flags, LinMessageFlags::Drop)) {
        str.append(",Drop");
    }

    if (HasFlag(flags, LinMessageFlags::Header)) {
        str.append(",Header");
    }

    if (HasFlag(flags, LinMessageFlags::Response)) {
        str.append(",Response");
    }

    if (HasFlag(flags, LinMessageFlags::WakeEvent)) {
        str.append(",WakeEvent");
    }

    if (HasFlag(flags, LinMessageFlags::SleepEvent)) {
        str.append(",SleepEvent");
    }

    if (HasFlag(flags, LinMessageFlags::EnhancedChecksum)) {
        str.append(",EnhancedChecksum");
    }

    if (HasFlag(flags, LinMessageFlags::TransferOnce)) {
        str.append(",TransferOnce");
    }

    if (HasFlag(flags, LinMessageFlags::ParityFailure)) {
        str.append(",ParityFailure");
    }

    if (HasFlag(flags, LinMessageFlags::Collision)) {
        str.append(",Collision");
    }

    if (HasFlag(flags, LinMessageFlags::NoResponse)) {
        str.append(",NoResponse");
    }

    if (!str.empty()) {
        str.erase(0, 1);
    }

    return str;
}

[[nodiscard]] LinControllerContainer::operator LinController() const {
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

[[nodiscard]] LinMessage::operator LinMessageContainer() const {
    LinMessageContainer message{};
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = flags;
    message.length = length;
    (void)memcpy(message.data.data(), data, length);
    return message;
}

[[nodiscard]] LinMessageContainer::operator LinMessage() const {
    CheckMaxLength();
    LinMessage message{};
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = flags;
    message.length = length;
    message.data = data.data();
    return message;
}

void LinMessageContainer::CheckMaxLength() const {
    if (length > LinMessageMaxLength) {
        throw std::runtime_error("LIN message data exceeds maximum length.");
    }
}

[[nodiscard]] std::string ToString(const LinMessage& message) {
    std::string str = "LIN Message { Timestamp: ";
    str.append(SimulationTimeToString(message.timestamp));
    str.append(", ControllerId: ");
    str.append(ToString(message.controllerId));
    str.append(", Id: ");
    str.append(ToString(message.id));
    str.append(", Length: ");
    str.append(std::to_string(message.length));
    str.append(", Data: ");
    str.append(DataToString(message.data, message.length, '-'));
    str.append(", Flags: ");
    str.append(ToString(message.flags));
    str.append(" }");
    return str;
}

[[nodiscard]] std::string ToString(const LinMessageContainer& message) {
    std::string str = "LIN Message { Timestamp: ";
    str.append(SimulationTimeToString(message.timestamp));
    str.append(", ControllerId: ");
    str.append(ToString(message.controllerId));
    str.append(", Id: ");
    str.append(ToString(message.id));
    str.append(", Length: ");
    str.append(std::to_string(message.length));
    str.append(", Data: ");
    str.append(DataToString(message.data.data(), message.length, '-'));
    str.append(", Flags: ");
    str.append(ToString(message.flags));
    str.append(" }");
    return str;
}

[[nodiscard]] std::string ToString(const LinController& controller) {
    std::string str = "LIN Controller { Id: ";
    str.append(ToString(controller.id));
    str.append(", QueueSize: ");
    str.append(std::to_string(controller.queueSize));
    str.append(", BitsPerSecond: ");
    str.append(std::to_string(controller.bitsPerSecond));
    str.append(", Type: ");
    str.append(ToString(controller.type));
    str.append(", Name: \"");
    str.append(controller.name);
    str.append("\", ChannelName: \"");
    str.append(controller.channelName);
    str.append("\", ClusterName: \"");
    str.append(controller.clusterName);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string ToString(const LinControllerContainer& controller) {
    std::string str = "LIN Controller { Id: ";
    str.append(ToString(controller.id));
    str.append(", QueueSize: ");
    str.append(std::to_string(controller.queueSize));
    str.append(", BitsPerSecond: ");
    str.append(std::to_string(controller.bitsPerSecond));
    str.append(", Type: ");
    str.append(ToString(controller.type));
    str.append(", Name: \"");
    str.append(controller.name);
    str.append("\", ChannelName: \"");
    str.append(controller.channelName);
    str.append("\", ClusterName: \"");
    str.append(controller.clusterName);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string ToString(const std::vector<LinControllerContainer>& controllers) {
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

[[nodiscard]] std::vector<LinController> Convert(const std::vector<LinControllerContainer>& controllers) {
    std::vector<LinController> linControllers;
    linControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        linControllers.push_back(static_cast<LinController>(controller));
    }

    return linControllers;
}

}  // namespace DsVeosCoSim
