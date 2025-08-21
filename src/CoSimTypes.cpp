// Copyright dSPACE GmbH. All rights reserved.

#include "DsVeosCoSim/CoSimTypes.h"

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
#include <vector>

namespace DsVeosCoSim {

extern void LogError(const std::string& message);

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

    return "<Invalid DataType>";
}

[[nodiscard]] std::string IoSignalToString(IoSignalId id,
                                           uint32_t length,
                                           DataType dataType,
                                           SizeKind sizeKind,
                                           const std::string& name) {
    std::string str = "IO Signal { Id: ";
    str.append(ToString(id));
    str.append(", Length: ");
    str.append(std::to_string(length));
    str.append(", DataType: ");
    str.append(ToString(dataType));
    str.append(", SizeKind: ");
    str.append(ToString(sizeKind));
    str.append(", Name: \"");
    str.append(name);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string CanControllerToString(BusControllerId id,
                                                uint32_t queueSize,
                                                uint64_t bitsPerSecond,
                                                uint64_t flexibleDataRateBitsPerSecond,
                                                const std::string& name,
                                                const std::string& channelName,
                                                const std::string& clusterName) {
    std::string str = "CAN Controller { Id: ";
    str.append(ToString(id));
    str.append(", QueueSize: ");
    str.append(std::to_string(queueSize));
    str.append(", BitsPerSecond: ");
    str.append(std::to_string(bitsPerSecond));
    str.append(", FlexibleDataRateBitsPerSecond: ");
    str.append(std::to_string(flexibleDataRateBitsPerSecond));
    str.append(", Name: \"");
    str.append(name);
    str.append("\", ChannelName: \"");
    str.append(channelName);
    str.append("\", ClusterName: \"");
    str.append(clusterName);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string CanMessageToString(SimulationTime timestamp,
                                             BusControllerId controllerId,
                                             BusMessageId id,
                                             uint32_t length,
                                             const uint8_t* data,
                                             CanMessageFlags flags) {
    std::string str = "CAN Message { Timestamp: ";
    str.append(SimulationTimeToString(timestamp));
    str.append(", ControllerId: ");
    str.append(ToString(controllerId));
    str.append(", Id: ");
    str.append(ToString(id));
    str.append(", Length: ");
    str.append(std::to_string(length));
    str.append(", Data: ");
    str.append(DataToString(data, length, '-'));
    str.append(", Flags: ");
    str.append(ToString(flags));
    str.append(" }");
    return str;
}

[[nodiscard]] Result CheckCanMessage(uint32_t length) {
    if (length > CanMessageMaxLength) {
        LogError("CAN message data exceeds maximum length.");
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] std::string EthControllerToString(BusControllerId id,
                                                uint32_t queueSize,
                                                uint64_t bitsPerSecond,
                                                std::array<uint8_t, EthAddressLength> macAddress,
                                                const std::string& name,
                                                const std::string& channelName,
                                                const std::string& clusterName) {
    std::string str = "ETH Controller { Id: ";
    str.append(ToString(id));
    str.append(", QueueSize: ");
    str.append(std::to_string(queueSize));
    str.append(", BitsPerSecond: ");
    str.append(std::to_string(bitsPerSecond));
    str.append(", MacAddress: [");
    str.append(DataToString(macAddress.data(), sizeof(macAddress), ':'));
    str.append("], Name: \"");
    str.append(name);
    str.append("\", ChannelName: \"");
    str.append(channelName);
    str.append("\", ClusterName: \"");
    str.append(clusterName);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string EthMessageToString(SimulationTime timestamp,
                                             BusControllerId controllerId,
                                             uint32_t length,
                                             const uint8_t* data,
                                             EthMessageFlags flags) {
    std::string str = "ETH Message { Timestamp: ";
    str.append(SimulationTimeToString(timestamp));
    str.append(", ControllerId: ");
    str.append(ToString(controllerId));
    str.append(", Length: ");
    str.append(std::to_string(length));
    str.append(", Data: ");
    str.append(DataToString(data, length, '-'));
    str.append(", Flags: ");
    str.append(ToString(flags));
    str.append(" }");
    return str;
}

[[nodiscard]] Result EthMessageCheck(uint32_t length) {
    if (length > EthMessageMaxLength) {
        LogError("Ethernet message data exceeds maximum length.");
        return Result::Error;
    }

    return Result::Ok;
}

[[nodiscard]] std::string LinControllerToString(BusControllerId id,
                                                uint32_t queueSize,
                                                uint64_t bitsPerSecond,
                                                LinControllerType type,
                                                const std::string& name,
                                                const std::string& channelName,
                                                const std::string& clusterName) {
    std::string str = "LIN Controller { Id: ";
    str.append(ToString(id));
    str.append(", QueueSize: ");
    str.append(std::to_string(queueSize));
    str.append(", BitsPerSecond: ");
    str.append(std::to_string(bitsPerSecond));
    str.append(", Type: ");
    str.append(ToString(type));
    str.append(", Name: \"");
    str.append(name);
    str.append("\", ChannelName: \"");
    str.append(channelName);
    str.append("\", ClusterName: \"");
    str.append(clusterName);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string LinMessageToString(SimulationTime timestamp,
                                             BusControllerId controllerId,
                                             BusMessageId id,
                                             uint32_t length,
                                             const uint8_t* data,
                                             LinMessageFlags flags) {
    std::string str = "LIN Message { Timestamp: ";
    str.append(SimulationTimeToString(timestamp));
    str.append(", ControllerId: ");
    str.append(ToString(controllerId));
    str.append(", Id: ");
    str.append(ToString(id));
    str.append(", Length: ");
    str.append(std::to_string(length));
    str.append(", Data: ");
    str.append(DataToString(data, length, '-'));
    str.append(", Flags: ");
    str.append(ToString(flags));
    str.append(" }");
    return str;
}

[[nodiscard]] Result LinMessageCheck(uint32_t length) {
    if (length > LinMessageMaxLength) {
        LogError("LIN message data exceeds maximum length.");
        return Result::Error;
    }

    return Result::Ok;
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

[[nodiscard]] const char* ToString(Result result) {
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

[[nodiscard]] const char* ToString(CoSimType coSimType) {
    switch (coSimType) {
        case CoSimType::Client:
            return "Client";
        case CoSimType::Server:
            return "Server";
    }

    return "<Invalid CoSimType>";
}

[[nodiscard]] const char* ToString(ConnectionKind connectionKind) {
    switch (connectionKind) {
        case ConnectionKind::Remote:
            return "Remote";
        case ConnectionKind::Local:
            return "Local";
    }

    return "<Invalid ConnectionKind>";
}

[[nodiscard]] const char* ToString(Command command) {
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

[[nodiscard]] const char* ToString(Severity severity) {
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

[[nodiscard]] const char* ToString(TerminateReason terminateReason) {
    switch (terminateReason) {
        case TerminateReason::Finished:
            return "Finished";
        case TerminateReason::Error:
            return "Error";
    }

    return "<Invalid TerminateReason>";
}

[[nodiscard]] const char* ToString(ConnectionState connectionState) {
    switch (connectionState) {
        case ConnectionState::Disconnected:
            return "Disconnected";
        case ConnectionState::Connected:
            return "Connected";
    }

    return "<Invalid ConnectionState>";
}

[[nodiscard]] size_t GetDataTypeSize(DataType dataType) {
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

[[nodiscard]] const char* ToString(DataType dataType) {
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

[[nodiscard]] const char* ToString(SizeKind sizeKind) {
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

[[nodiscard]] const char* ToString(SimulationState simulationState) {
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

[[nodiscard]] const char* ToString([[maybe_unused]] Mode mode) {
    return "<Unused>";
}

[[nodiscard]] std::string ToString(IoSignalId signalId) {
    return std::to_string(static_cast<uint32_t>(signalId));
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

[[nodiscard]] std::string IoSignal::ToString() const {
    return IoSignalToString(id, length, dataType, sizeKind, name);
}

[[nodiscard]] std::string IoSignalContainer::ToString() const {
    return IoSignalToString(id, length, dataType, sizeKind, name);
}

[[nodiscard]] IoSignal IoSignalContainer::Convert() const {
    IoSignal ioSignal{};
    ioSignal.id = id;
    ioSignal.length = length;
    ioSignal.dataType = dataType;
    ioSignal.sizeKind = sizeKind;
    ioSignal.name = name.c_str();
    return ioSignal;
}

[[nodiscard]] std::string ToString(const std::vector<IoSignalContainer>& signalContainers) {
    std::string str = "[";

    bool first = true;
    for (const IoSignalContainer& signalContainer : signalContainers) {
        if (first) {
            first = false;
        } else {
            str.append(", ");
        }

        str.append(signalContainer.ToString());
    }

    str.append("]");

    return str;
}

[[nodiscard]] std::vector<IoSignal> Convert(const std::vector<IoSignalContainer>& signalContainers) {
    std::vector<IoSignal> ioSignals;
    ioSignals.reserve(signalContainers.size());

    for (const auto& signalContainer : signalContainers) {
        ioSignals.push_back(signalContainer.Convert());
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

[[nodiscard]] std::string CanController::ToString() const {
    return CanControllerToString(id,
                                 queueSize,
                                 bitsPerSecond,
                                 flexibleDataRateBitsPerSecond,
                                 name,
                                 channelName,
                                 clusterName);
}

[[nodiscard]] std::string CanControllerContainer::ToString() const {
    return CanControllerToString(id,
                                 queueSize,
                                 bitsPerSecond,
                                 flexibleDataRateBitsPerSecond,
                                 name,
                                 channelName,
                                 clusterName);
}

[[nodiscard]] CanController CanControllerContainer::Convert() const {
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

[[nodiscard]] std::string ToString(const std::vector<CanControllerContainer>& controllerContainers) {
    std::string str = "[";

    bool first = true;
    for (const CanControllerContainer& controllerContainer : controllerContainers) {
        if (first) {
            first = false;
        } else {
            str.append(", ");
        }

        str.append(controllerContainer.ToString());
    }

    str.append("]");

    return str;
}

[[nodiscard]] std::vector<CanController> Convert(const std::vector<CanControllerContainer>& controllerContainers) {
    std::vector<CanController> controllers;
    controllers.reserve(controllerContainers.size());

    for (const auto& controllerContainer : controllerContainers) {
        controllers.push_back(controllerContainer.Convert());
    }

    return controllers;
}

[[nodiscard]] std::string CanMessage::ToString() const {
    return CanMessageToString(timestamp, controllerId, id, length, data, flags);
}

[[nodiscard]] Result CanMessage::Check() const {
    return CheckCanMessage(length);
}

void CanMessage::WriteTo(CanMessageContainer& messageContainer) const {
    messageContainer.timestamp = timestamp;
    messageContainer.controllerId = controllerId;
    messageContainer.id = id;
    messageContainer.flags = flags;
    messageContainer.length = length;
    (void)memcpy(messageContainer.data.data(), data, length);
}

[[nodiscard]] std::string CanMessageContainer::ToString() const {
    return CanMessageToString(timestamp, controllerId, id, length, data.data(), flags);
}

[[nodiscard]] Result CanMessageContainer::Check() const {
    return CheckCanMessage(length);
}

void CanMessageContainer::WriteTo(CanMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = flags;
    message.length = length;
    message.data = data.data();
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

[[nodiscard]] std::string EthController::ToString() const {
    return EthControllerToString(id, queueSize, bitsPerSecond, macAddress, name, channelName, clusterName);
}

[[nodiscard]] std::string EthControllerContainer::ToString() const {
    return EthControllerToString(id, queueSize, bitsPerSecond, macAddress, name, channelName, clusterName);
}

[[nodiscard]] EthController EthControllerContainer::Convert() const {
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

[[nodiscard]] std::string ToString(const std::vector<EthControllerContainer>& controllerContainers) {
    std::string str = "[";

    bool first = true;
    for (const EthControllerContainer& controllerContainer : controllerContainers) {
        if (first) {
            first = false;
        } else {
            str.append(", ");
        }

        str.append(controllerContainer.ToString());
    }

    str.append("]");

    return str;
}

[[nodiscard]] std::vector<EthController> Convert(const std::vector<EthControllerContainer>& controllerContainers) {
    std::vector<EthController> controllers;
    controllers.reserve(controllerContainers.size());

    for (const auto& controllerContainer : controllerContainers) {
        controllers.push_back(controllerContainer.Convert());
    }

    return controllers;
}

[[nodiscard]] std::string EthMessage::ToString() const {
    return EthMessageToString(timestamp, controllerId, length, data, flags);
}

[[nodiscard]] Result EthMessage::Check() const {
    return EthMessageCheck(length);
}

void EthMessage::WriteTo(EthMessageContainer& messageContainer) const {
    messageContainer.timestamp = timestamp;
    messageContainer.controllerId = controllerId;
    messageContainer.flags = flags;
    messageContainer.length = length;
    (void)memcpy(messageContainer.data.data(), data, length);
}

[[nodiscard]] std::string EthMessageContainer::ToString() const {
    return EthMessageToString(timestamp, controllerId, length, data.data(), flags);
}

[[nodiscard]] Result EthMessageContainer::Check() const {
    return EthMessageCheck(length);
}

void EthMessageContainer::WriteTo(EthMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.flags = flags;
    message.length = length;
    message.data = data.data();
}

[[nodiscard]] const char* ToString(LinControllerType type) {
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

[[nodiscard]] std::string LinController::ToString() const {
    return LinControllerToString(id, queueSize, bitsPerSecond, type, name, channelName, clusterName);
}

[[nodiscard]] std::string LinControllerContainer::ToString() const {
    return LinControllerToString(id, queueSize, bitsPerSecond, type, name, channelName, clusterName);
}

[[nodiscard]] LinController LinControllerContainer::Convert() const {
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

[[nodiscard]] std::string ToString(const std::vector<LinControllerContainer>& controllerContainers) {
    std::string str = "[";

    bool first = true;
    for (const LinControllerContainer& controllerContainer : controllerContainers) {
        if (first) {
            first = false;
        } else {
            str.append(", ");
        }

        str.append(controllerContainer.ToString());
    }

    str.append("]");

    return str;
}

[[nodiscard]] std::vector<LinController> Convert(const std::vector<LinControllerContainer>& controllerContainers) {
    std::vector<LinController> controllers;
    controllers.reserve(controllerContainers.size());

    for (const auto& controllerContainer : controllerContainers) {
        controllers.push_back(controllerContainer.Convert());
    }

    return controllers;
}

[[nodiscard]] std::string LinMessage::ToString() const {
    return LinMessageToString(timestamp, controllerId, id, length, data, flags);
}

[[nodiscard]] Result LinMessage::Check() const {
    return LinMessageCheck(length);
}

void LinMessage::WriteTo(LinMessageContainer& messageContainer) const {
    messageContainer.timestamp = timestamp;
    messageContainer.controllerId = controllerId;
    messageContainer.id = id;
    messageContainer.flags = flags;
    messageContainer.length = length;
    (void)memcpy(messageContainer.data.data(), data, length);
}

[[nodiscard]] std::string LinMessageContainer::ToString() const {
    return LinMessageToString(timestamp, controllerId, id, length, data.data(), flags);
}

[[nodiscard]] Result LinMessageContainer::Check() const {
    return LinMessageCheck(length);
}

void LinMessageContainer::WriteTo(LinMessage& message) const {
    message.timestamp = timestamp;
    message.controllerId = controllerId;
    message.id = id;
    message.flags = flags;
    message.length = length;
    message.data = data.data();
}

[[nodiscard]] const char* ToString(FrameKind frameKind) {
    switch (frameKind) {
        case FrameKind::Ping:
            return "Ping";
        case FrameKind::PingOk:
            return "PingOk";
        case FrameKind::Ok:
            return "Ok";
        case FrameKind::Error:
            return "Error";
        case FrameKind::Start:
            return "Start";
        case FrameKind::Stop:
            return "Stop";
        case FrameKind::Terminate:
            return "Terminate";
        case FrameKind::Pause:
            return "Pause";
        case FrameKind::Continue:
            return "Continue";
        case FrameKind::Step:
            return "Step";
        case FrameKind::StepOk:
            return "StepOk";
        case FrameKind::Connect:
            return "Connect";
        case FrameKind::ConnectOk:
            return "ConnectOk";
        case FrameKind::GetPort:
            return "GetPort";
        case FrameKind::GetPortOk:
            return "GetPortOk";
        case FrameKind::SetPort:
            return "SetPort";
        case FrameKind::UnsetPort:
            return "UnsetPort";
    }

    return "<Invalid FrameKind>";
}

}  // namespace DsVeosCoSim
