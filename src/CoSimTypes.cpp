// Copyright dSPACE GmbH. All rights reserved.

#include "DsVeosCoSim/CoSimTypes.h"

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

[[nodiscard]] std::string DataTypeValueToString(DataType dataType, uint32_t index, const void* value) {
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

[[nodiscard]] std::string DataToString(const uint8_t* data, size_t dataLength, char separator) {
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

[[nodiscard]] std::string SimulationTimeToString(SimulationTime simulationTime) {
    int64_t nanoseconds = simulationTime.count();
    std::string str = std::to_string(nanoseconds);

    size_t length = str.size();
    if (length < 10) {
        size_t countOfMissingZerosInFront = 10 - length;
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

[[nodiscard]] std::string_view ToString(Result result) noexcept {
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

[[nodiscard]] std::string_view ToString(CoSimType coSimType) noexcept {
    switch (coSimType) {
        case CoSimType::Client:
            return "Client";
        case CoSimType::Server:
            return "Server";
    }

    return "<Invalid CoSimType>";
}

[[nodiscard]] std::string_view ToString(ConnectionKind connectionKind) noexcept {
    switch (connectionKind) {
        case ConnectionKind::Remote:
            return "Remote";
        case ConnectionKind::Local:
            return "Local";
    }

    return "<Invalid ConnectionKind>";
}

[[nodiscard]] std::string_view ToString(Command command) noexcept {
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

[[nodiscard]] std::string_view ToString(Severity severity) noexcept {
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

[[nodiscard]] std::string_view ToString(TerminateReason terminateReason) noexcept {
    switch (terminateReason) {
        case TerminateReason::Finished:
            return "Finished";
        case TerminateReason::Error:
            return "Error";
    }

    return "<Invalid TerminateReason>";
}

[[nodiscard]] std::string_view ToString(ConnectionState connectionState) noexcept {
    switch (connectionState) {
        case ConnectionState::Disconnected:
            return "Disconnected";
        case ConnectionState::Connected:
            return "Connected";
    }

    return "<Invalid ConnectionState>";
}

[[nodiscard]] size_t GetDataTypeSize(DataType dataType) noexcept {
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

[[nodiscard]] std::string_view ToString(DataType dataType) noexcept {
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

[[nodiscard]] std::string_view ToString(SizeKind sizeKind) noexcept {
    switch (sizeKind) {
        case SizeKind::Fixed:
            return "Fixed";
        case SizeKind::Variable:
            return "Variable";
    }

    return "<Invalid SizeKind>";
}

[[nodiscard]] std::string ValueToString(DataType dataType, uint32_t length, const void* value) {
    std::ostringstream oss;
    for (uint32_t i = 0; i < length; i++) {
        if (i > 0) {
            oss << " ";
        }

        oss << DataTypeValueToString(dataType, i, value);
    }

    return oss.str();
}

[[nodiscard]] std::string_view ToString(SimulationState simulationState) noexcept {
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

[[nodiscard]] std::string_view ToString([[maybe_unused]] Mode mode) noexcept {
    return "<Unused>";
}

[[nodiscard]] std::string ToString(IoSignalId signalId) {
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

[[nodiscard]] std::string IoDataToString(const IoSignal& ioSignal, uint32_t length, const void* value) {
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
        if (first) {
            first = false;
        } else {
            str.append(", ");
        }

        str.append(ToString(signal));
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

[[nodiscard]] std::string ToString(BusControllerId busControllerId) {
    return std::to_string(static_cast<uint32_t>(busControllerId));
}

[[nodiscard]] std::string ToString(BusMessageId busMessageId) {
    return std::to_string(static_cast<uint32_t>(busMessageId));
}

[[nodiscard]] std::string ToString(CanMessageFlags flags) {
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
        if (first) {
            first = false;
        } else {
            str.append(", ");
        }

        str.append(ToString(controller));
    }

    str.append("]");

    return str;
}

[[nodiscard]] CanController Convert(const CanControllerContainer& controller) {
    CanController canController{};
    canController.id = controller.id;
    canController.queueSize = controller.queueSize;
    canController.bitsPerSecond = controller.bitsPerSecond;
    canController.flexibleDataRateBitsPerSecond = controller.flexibleDataRateBitsPerSecond;
    canController.name = controller.name.c_str();
    canController.channelName = controller.channelName.c_str();
    canController.clusterName = controller.clusterName.c_str();
    return canController;
}

[[nodiscard]] std::vector<CanController> Convert(const std::vector<CanControllerContainer>& controllers) {
    std::vector<CanController> canControllers;
    canControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        canControllers.push_back(Convert(controller));
    }

    return canControllers;
}

[[nodiscard]] CanMessageContainer Convert(const CanMessage& message) {
    CanMessageContainer canMessage{};
    canMessage.timestamp = message.timestamp;
    canMessage.controllerId = message.controllerId;
    canMessage.id = message.id;
    canMessage.flags = message.flags;
    canMessage.length = message.length;
    (void)memcpy(canMessage.data.data(), message.data, message.length);
    return canMessage;
}

[[nodiscard]] CanMessage Convert(const CanMessageContainer& message) {
    CanMessage canMessage{};
    canMessage.timestamp = message.timestamp;
    canMessage.controllerId = message.controllerId;
    canMessage.id = message.id;
    canMessage.flags = message.flags;
    canMessage.length = message.length;
    canMessage.data = message.data.data();
    return canMessage;
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
        if (first) {
            first = false;
        } else {
            str.append(", ");
        }

        str.append(ToString(controller));
    }

    str.append("]");

    return str;
}

[[nodiscard]] EthController Convert(const EthControllerContainer& controller) {
    EthController ethController{};
    ethController.id = controller.id;
    ethController.queueSize = controller.queueSize;
    ethController.bitsPerSecond = controller.bitsPerSecond;
    ethController.macAddress = controller.macAddress;
    ethController.name = controller.name.c_str();
    ethController.channelName = controller.channelName.c_str();
    ethController.clusterName = controller.clusterName.c_str();
    return ethController;
}

[[nodiscard]] std::vector<EthController> Convert(const std::vector<EthControllerContainer>& controllers) {
    std::vector<EthController> ethControllers;
    ethControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        ethControllers.push_back(Convert(controller));
    }

    return ethControllers;
}

[[nodiscard]] EthMessageContainer Convert(const EthMessage& message) {
    EthMessageContainer ethMessage{};
    ethMessage.timestamp = message.timestamp;
    ethMessage.controllerId = message.controllerId;
    ethMessage.flags = message.flags;
    ethMessage.length = message.length;
    (void)memcpy(ethMessage.data.data(), message.data, message.length);
    return ethMessage;
}

[[nodiscard]] EthMessage Convert(const EthMessageContainer& message) {
    EthMessage ethMessage{};
    ethMessage.timestamp = message.timestamp;
    ethMessage.controllerId = message.controllerId;
    ethMessage.flags = message.flags;
    ethMessage.length = message.length;
    ethMessage.data = message.data.data();
    return ethMessage;
}

[[nodiscard]] std::string_view ToString(LinControllerType type) noexcept {
    switch (type) {
        case LinControllerType::Responder:
            return "Responder";
        case LinControllerType::Commander:
            return "Commander";
    }

    return "<Invalid LinControllerType>";
}

[[nodiscard]] std::string ToString(LinMessageFlags flags) {
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
        if (first) {
            first = false;
        } else {
            str.append(", ");
        }

        str.append(ToString(controller));
    }

    str.append("]");

    return str;
}

[[nodiscard]] LinController Convert(const LinControllerContainer& controller) {
    LinController linController{};
    linController.id = controller.id;
    linController.queueSize = controller.queueSize;
    linController.bitsPerSecond = controller.bitsPerSecond;
    linController.type = controller.type;
    linController.name = controller.name.c_str();
    linController.channelName = controller.channelName.c_str();
    linController.clusterName = controller.clusterName.c_str();
    return linController;
}

[[nodiscard]] std::vector<LinController> Convert(const std::vector<LinControllerContainer>& controllers) {
    std::vector<LinController> linControllers;
    linControllers.reserve(controllers.size());

    for (const auto& controller : controllers) {
        linControllers.push_back(Convert(controller));
    }

    return linControllers;
}

[[nodiscard]] LinMessageContainer Convert(const LinMessage& message) {
    LinMessageContainer linMessage{};
    linMessage.timestamp = message.timestamp;
    linMessage.controllerId = message.controllerId;
    linMessage.id = message.id;
    linMessage.flags = message.flags;
    linMessage.length = message.length;
    (void)memcpy(linMessage.data.data(), message.data, message.length);
    return linMessage;
}

[[nodiscard]] LinMessage Convert(const LinMessageContainer& message) {
    LinMessage linMessage{};
    linMessage.timestamp = message.timestamp;
    linMessage.controllerId = message.controllerId;
    linMessage.id = message.id;
    linMessage.flags = message.flags;
    linMessage.length = message.length;
    linMessage.data = message.data.data();
    return linMessage;
}

}  // namespace DsVeosCoSim
