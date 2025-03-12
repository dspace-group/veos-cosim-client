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

[[nodiscard]] std::string ToString(const Result result) {
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

[[nodiscard]] std::string ToString(const CoSimType coSimType) {
    switch (coSimType) {
        case CoSimType::Client:
            return "Client";
        case CoSimType::Server:
            return "Server";
    }

    return "<Invalid CoSimType>";
}

[[nodiscard]] std::string ToString(const ConnectionKind connectionKind) {
    switch (connectionKind) {
        case ConnectionKind::Remote:
            return "Remote";
        case ConnectionKind::Local:
            return "Local";
    }

    return "<Invalid ConnectionKind>";
}

[[nodiscard]] std::string ToString(const Command command) {
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

[[nodiscard]] std::string ToString(const Severity severity) {
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

[[nodiscard]] std::string ToString(const TerminateReason terminateReason) {
    switch (terminateReason) {
        case TerminateReason::Finished:
            return "Finished";
        case TerminateReason::Error:
            return "Error";
    }

    return "<Invalid TerminateReason>";
}

[[nodiscard]] std::string ToString(const ConnectionState connectionState) {
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

[[nodiscard]] std::string ToString(const DataType dataType) {
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

[[nodiscard]] std::string ToString(const SizeKind sizeKind) {
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

[[nodiscard]] std::string ToString(const SimulationState simulationState) {
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

[[nodiscard]] std::string ToString([[maybe_unused]] const Mode mode) {
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
    return "IO Data { Id: " + ToString(ioSignal.id) + ", Length: " + std::to_string(length) +
           ", Data: " + ValueToString(ioSignal.dataType, length, value) + " }";
}

[[nodiscard]] std::string ToString(const IoSignal& signal) {
    std::string str = "IO Signal { Id: " + ToString(signal.id) + ", Length: " + std::to_string(signal.length) +
                      ", DataType: " + ToString(signal.dataType) + ", SizeKind: " + ToString(signal.sizeKind) +
                      ", Name: \"" + signal.name + "\"}";

    return str;
}

[[nodiscard]] std::string ToString(const IoSignalContainer& signal) {
    std::string str = "IO Signal { Id: " + ToString(signal.id) + ", Length: " + std::to_string(signal.length) +
                      ", DataType: " + ToString(signal.dataType) + ", SizeKind: " + ToString(signal.sizeKind) +
                      ", Name: \"" + signal.name + "\" }";

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
        throw CoSimException("CAN message data exceeds maximum length.");
    }
}

void CanMessageContainer::CheckFlags() const {
    if (!HasFlag(flags, CanMessageFlags::FlexibleDataRateFormat)) {
        if (length > 8) {
            throw CoSimException("CAN message flags invalid. A DLC > 8 requires the flexible data rate format flag.");
        }

        if (HasFlag(flags, CanMessageFlags::BitRateSwitch)) {
            throw CoSimException(
                "CAN message flags invalid. A bit rate switch flag requires the flexible data rate format flag.");
        }
    }
}

[[nodiscard]] std::string ToString(const CanMessage& message) {
    return "CAN Message { Timestamp: " + SimulationTimeToString(message.timestamp) +
           ", ControllerId: " + ToString(message.controllerId) + ", Id: " + ToString(message.id) +
           ", Length: " + std::to_string(message.length) +
           ", Data: " + DataToString(message.data, message.length, '-') + ", Flags: " + ToString(message.flags) + " }";
}

[[nodiscard]] std::string ToString(const CanMessageContainer& message) {
    return "CAN Message { Timestamp: " + SimulationTimeToString(message.timestamp) +
           ", ControllerId: " + ToString(message.controllerId) + ", Id: " + ToString(message.id) +
           ", Length: " + std::to_string(message.length) +
           ", Data: " + DataToString(message.data.data(), message.length, '-') + ", Flags: " + ToString(message.flags) +
           " }";
}

[[nodiscard]] std::string ToString(const CanController& controller) {
    return "CAN Controller { Id: " + ToString(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
           ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) +
           ", FlexibleDataRateBitsPerSecond: " + std::to_string(controller.flexibleDataRateBitsPerSecond) +
           ", Name: \"" + controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
           controller.clusterName + "\" }";
}

[[nodiscard]] std::string ToString(const CanControllerContainer& controller) {
    return "CAN Controller { Id: " + ToString(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
           ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) +
           ", FlexibleDataRateBitsPerSecond: " + std::to_string(controller.flexibleDataRateBitsPerSecond) +
           ", Name: \"" + controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
           controller.clusterName + "\" }";
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
        throw CoSimException("Ethernet message data exceeds maximum length.");
    }
}

[[nodiscard]] std::string ToString(const EthMessage& message) {
    return "ETH Message { Timestamp: " + SimulationTimeToString(message.timestamp) +
           ", ControllerId: " + ToString(message.controllerId) + ", Length: " + std::to_string(message.length) +
           ", Data: " + DataToString(message.data, message.length, '-') + ", Flags: " + ToString(message.flags) + " }";
}

[[nodiscard]] std::string ToString(const EthMessageContainer& message) {
    return "ETH Message { Timestamp: " + SimulationTimeToString(message.timestamp) +
           ", ControllerId: " + ToString(message.controllerId) + ", Length: " + std::to_string(message.length) +
           ", Data: " + DataToString(message.data.data(), message.length, '-') + ", Flags: " + ToString(message.flags) +
           " }";
}

[[nodiscard]] std::string ToString(const EthController& controller) {
    return "ETH Controller { Id: " + ToString(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
           ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) + ", MacAddress: [" +
           DataToString(controller.macAddress.data(), sizeof(controller.macAddress), ':') + "], Name: \"" +
           controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
           controller.clusterName + "\" }";
}

[[nodiscard]] std::string ToString(const EthControllerContainer& controller) {
    return "ETH Controller { Id: " + ToString(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
           ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) + ", MacAddress: [" +
           DataToString(controller.macAddress.data(), sizeof(controller.macAddress), ':') + "], Name: \"" +
           controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
           controller.clusterName + "\" }";
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

[[nodiscard]] std::string ToString(const LinControllerType type) {
    switch (type) {
        case LinControllerType::Responder:
            return "Responder";
        case LinControllerType::Commander:
            return "Commander";
    }

    return "<Invalid LinControllerType>";
}

[[nodiscard]] std::string ToString(const LinMessageFlags flags) {
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
        throw CoSimException("LIN message data exceeds maximum length.");
    }
}

[[nodiscard]] std::string ToString(const LinMessage& message) {
    return "LIN Message { Timestamp: " + SimulationTimeToString(message.timestamp) +
           ", ControllerId: " + ToString(message.controllerId) + ", Id: " + ToString(message.id) +
           ", Length: " + std::to_string(message.length) +
           ", Data: " + DataToString(message.data, message.length, '-') + ", Flags: " + ToString(message.flags) + " }";
}

[[nodiscard]] std::string ToString(const LinMessageContainer& message) {
    return "LIN Message { Timestamp: " + SimulationTimeToString(message.timestamp) +
           ", ControllerId: " + ToString(message.controllerId) + ", Id: " + ToString(message.id) +
           ", Length: " + std::to_string(message.length) +
           ", Data: " + DataToString(message.data.data(), message.length, '-') + ", Flags: " + ToString(message.flags) +
           " }";
}

[[nodiscard]] std::string ToString(const LinController& controller) {
    return "LIN Controller { Id: " + ToString(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
           ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) + ", Type: " + ToString(controller.type) +
           ", Name: \"" + controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
           controller.clusterName + "\" }";
}

[[nodiscard]] std::string ToString(const LinControllerContainer& controller) {
    return "LIN Controller { Id: " + ToString(controller.id) + ", QueueSize: " + std::to_string(controller.queueSize) +
           ", BitsPerSecond: " + std::to_string(controller.bitsPerSecond) + ", Type: " + ToString(controller.type) +
           ", Name: \"" + controller.name + "\", ChannelName: \"" + controller.channelName + "\", ClusterName: \"" +
           controller.clusterName + "\" }";
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
