// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "DsVeosCoSim/CoSimTypes.h"

#include <array>
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

[[nodiscard]] bool Equals(const std::string& first, const std::string& second) {
    if (first.length() != second.length()) {
        return false;
    }

    return strcmp(first.c_str(), second.c_str()) == 0;
}

[[nodiscard]] bool Equals(const void* expected, const void* actual, size_t size) {
    const auto* expectedBytes = static_cast<const uint8_t*>(expected);
    const auto* actualBytes = static_cast<const uint8_t*>(actual);
    for (size_t i = 0; i < size; i++) {
        if (expectedBytes[i] != actualBytes[i]) {
            return false;
        }
    }

    return true;
}

template <typename T, size_t TSize>
[[nodiscard]] bool Equals(const std::array<T, TSize>& expected, const std::array<T, TSize>& actual) {
    for (size_t i = 0; i < TSize; i++) {
        if (expected[i] != actual[i]) {
            return false;
        }
    }

    return true;
}

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

[[nodiscard]] std::string IoSignalToString(IoSignalId signalId, uint32_t length, DataType dataType, SizeKind sizeKind, const std::string& name) {
    std::string str = "IO Signal { Id: ";
    str.append(ToString(signalId));
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

[[nodiscard]] std::string CanControllerToString(BusControllerId controllerId,
                                                uint32_t queueSize,
                                                uint64_t bitsPerSecond,
                                                uint64_t flexibleDataRateBitsPerSecond,
                                                const std::string& name,
                                                const std::string& channelName,
                                                const std::string& clusterName) {
    std::string str = "CAN Controller { Id: ";
    str.append(ToString(controllerId));
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
                                             BusMessageId messageId,
                                             uint32_t length,
                                             const uint8_t* data,
                                             CanMessageFlags flags) {
    std::string str = "CAN Message { Timestamp: ";
    str.append(ToString(timestamp));
    str.append(", ControllerId: ");
    str.append(ToString(controllerId));
    str.append(", Id: ");
    str.append(ToString(messageId));
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
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

[[nodiscard]] std::string EthControllerToString(BusControllerId controllerId,
                                                uint32_t queueSize,
                                                uint64_t bitsPerSecond,
                                                std::array<uint8_t, EthAddressLength> macAddress,
                                                const std::string& name,
                                                const std::string& channelName,
                                                const std::string& clusterName) {
    std::string str = "ETH Controller { Id: ";
    str.append(ToString(controllerId));
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
    str.append(ToString(timestamp));
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
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

[[nodiscard]] std::string LinControllerToString(BusControllerId controllerId,
                                                uint32_t queueSize,
                                                uint64_t bitsPerSecond,
                                                LinControllerType type,
                                                const std::string& name,
                                                const std::string& channelName,
                                                const std::string& clusterName) {
    std::string str = "LIN Controller { Id: ";
    str.append(ToString(controllerId));
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
                                             BusMessageId messageId,
                                             uint32_t length,
                                             const uint8_t* data,
                                             LinMessageFlags flags) {
    std::string str = "LIN Message { Timestamp: ";
    str.append(ToString(timestamp));
    str.append(", ControllerId: ");
    str.append(ToString(controllerId));
    str.append(", Id: ");
    str.append(ToString(messageId));
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
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

[[nodiscard]] std::string FrControllerToString(BusControllerId controllerId,
                                               uint32_t queueSize,
                                               uint64_t bitsPerSecond,
                                               const std::string& name,
                                               const std::string& channelName,
                                               const std::string& clusterName) {
    std::string str = "FLEXRAY Controller { Id: ";
    str.append(ToString(controllerId));
    str.append(", QueueSize: ");
    str.append(std::to_string(queueSize));
    str.append(", BitsPerSecond: ");
    str.append(std::to_string(bitsPerSecond));
    str.append(", Name: \"");
    str.append(name);
    str.append("\", ChannelName: \"");
    str.append(channelName);
    str.append("\", ClusterName: \"");
    str.append(clusterName);
    str.append("\" }");
    return str;
}

[[nodiscard]] std::string FrMessageToString(SimulationTime timestamp,
                                            BusControllerId controllerId,
                                            BusMessageId messageId,
                                            uint32_t length,
                                            const uint8_t* data,
                                            FrMessageFlags flags) {
    std::string str = "FLEXRAY Message { Timestamp: ";
    str.append(ToString(timestamp));
    str.append(", ControllerId: ");
    str.append(ToString(controllerId));
    str.append(", Id: ");
    str.append(ToString(messageId));
    str.append(", Length: ");
    str.append(std::to_string(length));
    str.append(", Data: ");
    str.append(DataToString(data, length, '-'));
    str.append(", Flags: ");
    str.append(ToString(flags));
    str.append(" }");
    return str;
}

[[nodiscard]] Result FrMessageCheck(uint32_t length) {
    if (length > FrMessageMaxLength) {
        LogError("FLEXRAY message data exceeds maximum length.");
        return Result::InvalidArgument;
    }

    return Result::Ok;
}

}  // namespace

[[nodiscard]] std::string ToString(SimulationTime simulationTime) {
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

[[nodiscard]] std::string ToString(IoSignalId ioSignalId) {
    return std::to_string(static_cast<uint32_t>(ioSignalId));
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

[[nodiscard]] std::string ToString(BusControllerId busControllerId) {
    return std::to_string(static_cast<uint32_t>(busControllerId));
}

[[nodiscard]] std::string ToString(BusMessageId busMessageId) {
    return std::to_string(static_cast<uint32_t>(busMessageId));
}

[[nodiscard]] const char* ToString(LinControllerType linControllerType) {
    switch (linControllerType) {
        case LinControllerType::Responder:
            return "Responder";
        case LinControllerType::Commander:
            return "Commander";
    }

    return "<Invalid LinControllerType>";
}

[[nodiscard]] std::string ToString(CanMessageFlags canMessageFlags) {
    std::string str;

    if (HasFlag(canMessageFlags, CanMessageFlags::Loopback)) {
        str.append(",Loopback");
    }

    if (HasFlag(canMessageFlags, CanMessageFlags::Error)) {
        str.append(",Error");
    }

    if (HasFlag(canMessageFlags, CanMessageFlags::Drop)) {
        str.append(",Drop");
    }

    if (HasFlag(canMessageFlags, CanMessageFlags::ExtendedId)) {
        str.append(",ExtendedId");
    }

    if (HasFlag(canMessageFlags, CanMessageFlags::BitRateSwitch)) {
        str.append(",BitRateSwitch");
    }

    if (HasFlag(canMessageFlags, CanMessageFlags::FlexibleDataRateFormat)) {
        str.append(",FlexibleDataRateFormat");
    }

    if (!str.empty()) {
        str.erase(0, 1);
    }

    return str;
}

[[nodiscard]] std::string ToString(EthMessageFlags ethMessageFlags) {
    std::string str;

    if (HasFlag(ethMessageFlags, EthMessageFlags::Loopback)) {
        str.append(",Loopback");
    }

    if (HasFlag(ethMessageFlags, EthMessageFlags::Error)) {
        str.append(",Error");
    }

    if (HasFlag(ethMessageFlags, EthMessageFlags::Drop)) {
        str.append(",Drop");
    }

    if (!str.empty()) {
        str.erase(0, 1);
    }

    return str;
}

[[nodiscard]] std::string ToString(LinMessageFlags linMessageFlags) {
    std::string str;

    if (HasFlag(linMessageFlags, LinMessageFlags::Loopback)) {
        str.append(",Loopback");
    }

    if (HasFlag(linMessageFlags, LinMessageFlags::Error)) {
        str.append(",Error");
    }

    if (HasFlag(linMessageFlags, LinMessageFlags::Drop)) {
        str.append(",Drop");
    }

    if (HasFlag(linMessageFlags, LinMessageFlags::Header)) {
        str.append(",Header");
    }

    if (HasFlag(linMessageFlags, LinMessageFlags::Response)) {
        str.append(",Response");
    }

    if (HasFlag(linMessageFlags, LinMessageFlags::WakeEvent)) {
        str.append(",WakeEvent");
    }

    if (HasFlag(linMessageFlags, LinMessageFlags::SleepEvent)) {
        str.append(",SleepEvent");
    }

    if (HasFlag(linMessageFlags, LinMessageFlags::EnhancedChecksum)) {
        str.append(",EnhancedChecksum");
    }

    if (HasFlag(linMessageFlags, LinMessageFlags::TransferOnce)) {
        str.append(",TransferOnce");
    }

    if (HasFlag(linMessageFlags, LinMessageFlags::ParityFailure)) {
        str.append(",ParityFailure");
    }

    if (HasFlag(linMessageFlags, LinMessageFlags::Collision)) {
        str.append(",Collision");
    }

    if (HasFlag(linMessageFlags, LinMessageFlags::NoResponse)) {
        str.append(",NoResponse");
    }

    if (!str.empty()) {
        str.erase(0, 1);
    }

    return str;
}

[[nodiscard]] std::string ToString(FrMessageFlags frMessageFlags) {
    std::string str;

    if (HasFlag(frMessageFlags, FrMessageFlags::Loopback)) {
        str.append(",Loopback");
    }
    if (HasFlag(frMessageFlags, FrMessageFlags::Error)) {
        str.append(",Error");
    }
    if (HasFlag(frMessageFlags, FrMessageFlags::Drop)) {
        str.append(",Drop");
    }
    if (HasFlag(frMessageFlags, FrMessageFlags::Startup)) {
        str.append(",Startup");
    }
    if (HasFlag(frMessageFlags, FrMessageFlags::SyncFrame)) {
        str.append(",SyncFrame");
    }
    if (HasFlag(frMessageFlags, FrMessageFlags::NullFrame)) {
        str.append(",NullFrame");
    }
    if (HasFlag(frMessageFlags, FrMessageFlags::PayloadPreamble)) {
        str.append(",PayloadPreamble");
    }
    if (HasFlag(frMessageFlags, FrMessageFlags::TransferOnce)) {
        str.append(",TransferOnce");
    }
    if (HasFlag(frMessageFlags, FrMessageFlags::ChannelA)) {
        str.append(",ChannelA");
    }
    if (HasFlag(frMessageFlags, FrMessageFlags::ChannelB)) {
        str.append(",ChannelB");
    }

    return str;
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

[[nodiscard]] std::string IoSignal::ToString() const {
    return IoSignalToString(id, length, dataType, sizeKind, name);
}

[[nodiscard]] std::string IoSignalContainer::ToString() const {
    return IoSignalToString(id, length, dataType, sizeKind, name);
}

[[nodiscard]] std::string CanController::ToString() const {
    return CanControllerToString(id, queueSize, bitsPerSecond, flexibleDataRateBitsPerSecond, name, channelName, clusterName);
}

[[nodiscard]] std::string CanControllerContainer::ToString() const {
    return CanControllerToString(id, queueSize, bitsPerSecond, flexibleDataRateBitsPerSecond, name, channelName, clusterName);
}

[[nodiscard]] std::string CanMessage::ToString() const {
    return CanMessageToString(timestamp, controllerId, id, length, data, flags);
}

[[nodiscard]] std::string CanMessageContainer::ToString() const {
    return CanMessageToString(timestamp, controllerId, id, length, data.data(), flags);
}

[[nodiscard]] std::string EthController::ToString() const {
    return EthControllerToString(id, queueSize, bitsPerSecond, macAddress, name, channelName, clusterName);
}

[[nodiscard]] std::string EthControllerContainer::ToString() const {
    return EthControllerToString(id, queueSize, bitsPerSecond, macAddress, name, channelName, clusterName);
}

[[nodiscard]] std::string EthMessage::ToString() const {
    return EthMessageToString(timestamp, controllerId, length, data, flags);
}

[[nodiscard]] std::string EthMessageContainer::ToString() const {
    return EthMessageToString(timestamp, controllerId, length, data.data(), flags);
}

[[nodiscard]] std::string LinController::ToString() const {
    return LinControllerToString(id, queueSize, bitsPerSecond, type, name, channelName, clusterName);
}

[[nodiscard]] std::string LinControllerContainer::ToString() const {
    return LinControllerToString(id, queueSize, bitsPerSecond, type, name, channelName, clusterName);
}

[[nodiscard]] std::string LinMessage::ToString() const {
    return LinMessageToString(timestamp, controllerId, id, length, data, flags);
}

[[nodiscard]] std::string LinMessageContainer::ToString() const {
    return LinMessageToString(timestamp, controllerId, id, length, data.data(), flags);
}

[[nodiscard]] std::string FrController::ToString() const {
    return FrControllerToString(id, queueSize, bitsPerSecond, name, channelName, clusterName);
}

[[nodiscard]] std::string FrControllerContainer::ToString() const {
    return FrControllerToString(id, queueSize, bitsPerSecond, name, channelName, clusterName);
}

[[nodiscard]] std::string FrMessage::ToString() const {
    return FrMessageToString(timestamp, controllerId, id, length, data, flags);
}

[[nodiscard]] std::string FrMessageContainer::ToString() const {
    return FrMessageToString(timestamp, controllerId, id, length, data.data(), flags);
}

[[nodiscard]] std::string ToString(const std::vector<IoSignalContainer>& ioSignalContainers) {
    std::string str = "[";

    bool first = true;
    for (const IoSignalContainer& signalContainer : ioSignalContainers) {
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

[[nodiscard]] std::string ToString(const std::vector<CanControllerContainer>& canControllerContainers) {
    std::string str = "[";

    bool first = true;
    for (const CanControllerContainer& canControllerContainer : canControllerContainers) {
        if (first) {
            first = false;
        } else {
            str.append(", ");
        }

        str.append(canControllerContainer.ToString());
    }

    str.append("]");

    return str;
}

[[nodiscard]] std::string ToString(const std::vector<EthControllerContainer>& ethControllerContainers) {
    std::string str = "[";

    bool first = true;
    for (const EthControllerContainer& ethControllerContainer : ethControllerContainers) {
        if (first) {
            first = false;
        } else {
            str.append(", ");
        }

        str.append(ethControllerContainer.ToString());
    }

    str.append("]");

    return str;
}

[[nodiscard]] std::string ToString(const std::vector<LinControllerContainer>& linControllerContainers) {
    std::string str = "[";

    bool first = true;
    for (const LinControllerContainer& linControllerContainer : linControllerContainers) {
        if (first) {
            first = false;
        } else {
            str.append(", ");
        }

        str.append(linControllerContainer.ToString());
    }

    str.append("]");

    return str;
}

[[nodiscard]] std::string ToString(const std::vector<FrControllerContainer>& frControllerContainers) {
    std::string str = "[";

    bool first = true;
    for (const FrControllerContainer& frControllerContainer : frControllerContainers) {
        if (first) {
            first = false;
        } else {
            str.append(", ");
        }

        str.append(frControllerContainer.ToString());
    }

    str.append("]");

    return str;
}

std::ostream& operator<<(std::ostream& stream, SimulationTime simulationTime) {
    return stream << ToString(simulationTime);
}

std::ostream& operator<<(std::ostream& stream, Result result) {
    return stream << ToString(result);
}

std::ostream& operator<<(std::ostream& stream, CoSimType coSimType) {
    return stream << ToString(coSimType);
}

std::ostream& operator<<(std::ostream& stream, ConnectionKind connectionKind) {
    return stream << ToString(connectionKind);
}

std::ostream& operator<<(std::ostream& stream, Command command) {
    return stream << ToString(command);
}

std::ostream& operator<<(std::ostream& stream, Severity severity) {
    return stream << ToString(severity);
}

std::ostream& operator<<(std::ostream& stream, TerminateReason terminateReason) {
    return stream << ToString(terminateReason);
}

std::ostream& operator<<(std::ostream& stream, ConnectionState connectionState) {
    return stream << ToString(connectionState);
}

std::ostream& operator<<(std::ostream& stream, SimulationState simulationState) {
    return stream << ToString(simulationState);
}

std::ostream& operator<<(std::ostream& stream, Mode mode) {
    return stream << ToString(mode);
}

std::ostream& operator<<(std::ostream& stream, IoSignalId ioSignalId) {
    return stream << ToString(ioSignalId);
}

std::ostream& operator<<(std::ostream& stream, DataType dataType) {
    return stream << ToString(dataType);
}

std::ostream& operator<<(std::ostream& stream, SizeKind sizeKind) {
    return stream << ToString(sizeKind);
}

std::ostream& operator<<(std::ostream& stream, BusControllerId busControllerId) {
    return stream << ToString(busControllerId);
}

std::ostream& operator<<(std::ostream& stream, BusMessageId busMessageId) {
    return stream << ToString(busMessageId);
}

std::ostream& operator<<(std::ostream& stream, LinControllerType linControllerType) {
    return stream << ToString(linControllerType);
}

std::ostream& operator<<(std::ostream& stream, CanMessageFlags canMessageFlags) {
    return stream << ToString(canMessageFlags);
}

std::ostream& operator<<(std::ostream& stream, EthMessageFlags ethMessageFlags) {
    return stream << ToString(ethMessageFlags);
}

std::ostream& operator<<(std::ostream& stream, LinMessageFlags linMessageFlags) {
    return stream << ToString(linMessageFlags);
}

std::ostream& operator<<(std::ostream& stream, FrMessageFlags frMessageFlags) {
    return stream << ToString(frMessageFlags);
}

std::ostream& operator<<(std::ostream& stream, FrameKind frameKind) {
    return stream << ToString(frameKind);
}

std::ostream& operator<<(std::ostream& stream, const IoSignal& ioSignal) {
    return stream << ioSignal.ToString();
}

std::ostream& operator<<(std::ostream& stream, const IoSignalContainer& ioSignalContainer) {
    return stream << ioSignalContainer.ToString();
}

std::ostream& operator<<(std::ostream& stream, const CanController& canController) {
    return stream << canController.ToString();
}

std::ostream& operator<<(std::ostream& stream, const CanControllerContainer& canControllerContainer) {
    return stream << canControllerContainer.ToString();
}

std::ostream& operator<<(std::ostream& stream, const CanMessage& canMessage) {
    return stream << canMessage.ToString();
}

std::ostream& operator<<(std::ostream& stream, const CanMessageContainer& canMessageContainer) {
    return stream << canMessageContainer.ToString();
}

std::ostream& operator<<(std::ostream& stream, const EthController& ethController) {
    return stream << ethController.ToString();
}

std::ostream& operator<<(std::ostream& stream, const EthControllerContainer& ethControllerContainer) {
    return stream << ethControllerContainer.ToString();
}

std::ostream& operator<<(std::ostream& stream, const EthMessage& ethMessage) {
    return stream << ethMessage.ToString();
}

std::ostream& operator<<(std::ostream& stream, const EthMessageContainer& ethMessageContainer) {
    return stream << ethMessageContainer.ToString();
}

std::ostream& operator<<(std::ostream& stream, const LinController& linController) {
    return stream << linController.ToString();
}

std::ostream& operator<<(std::ostream& stream, const LinControllerContainer& frControllerContainer) {
    return stream << frControllerContainer.ToString();
}

std::ostream& operator<<(std::ostream& stream, const LinMessage& linMessage) {
    return stream << linMessage.ToString();
}

std::ostream& operator<<(std::ostream& stream, const LinMessageContainer& linMessageContainer) {
    return stream << linMessageContainer.ToString();
}

std::ostream& operator<<(std::ostream& stream, const FrController& frController) {
    return stream << frController.ToString();
}

std::ostream& operator<<(std::ostream& stream, const FrControllerContainer& frControllerContainer) {
    return stream << frControllerContainer.ToString();
}

std::ostream& operator<<(std::ostream& stream, const FrMessage& frMessage) {
    return stream << frMessage.ToString();
}

std::ostream& operator<<(std::ostream& stream, const FrMessageContainer& frMessageContainer) {
    return stream << frMessageContainer.ToString();
}

std::ostream& operator<<(std::ostream& stream, const std::vector<IoSignalContainer>& ioSignalContainers) {
    return stream << ToString(ioSignalContainers);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<CanControllerContainer>& canControllerContainers) {
    return stream << ToString(canControllerContainers);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<EthControllerContainer>& ethControllerContainers) {
    return stream << ToString(ethControllerContainers);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<LinControllerContainer>& linControllerContainers) {
    return stream << ToString(linControllerContainers);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<FrControllerContainer>& frControllerContainers) {
    return stream << ToString(frControllerContainers);
}

[[nodiscard]] bool operator==(const IoSignal& first, const IoSignal& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (first.dataType != second.dataType) {
        return false;
    }

    if (first.sizeKind != second.sizeKind) {
        return false;
    }

    if (strcmp(first.name, second.name) != 0) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const IoSignalContainer& first, const IoSignalContainer& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (first.dataType != second.dataType) {
        return false;
    }

    if (first.sizeKind != second.sizeKind) {
        return false;
    }

    if (Equals(first.name, second.name)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const CanController& first, const CanController& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (first.flexibleDataRateBitsPerSecond != second.flexibleDataRateBitsPerSecond) {
        return false;
    }

    if (strcmp(first.name, second.name) != 0) {
        return false;
    }

    if (strcmp(first.channelName, second.channelName) != 0) {
        return false;
    }

    if (strcmp(first.clusterName, second.clusterName) != 0) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const CanControllerContainer& first, const CanControllerContainer& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (first.flexibleDataRateBitsPerSecond != second.flexibleDataRateBitsPerSecond) {
        return false;
    }

    if (!Equals(first.name, second.name)) {
        return false;
    }

    if (!Equals(first.channelName, second.channelName)) {
        return false;
    }

    if (!Equals(first.clusterName, second.clusterName)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const CanMessage& first, const CanMessage& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.id != second.id) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data, second.data, first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const CanMessageContainer& first, const CanMessageContainer& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.id != second.id) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data.data(), second.data.data(), first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const EthController& first, const EthController& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (!Equals(first.macAddress, second.macAddress)) {
        return false;
    }

    if (strcmp(first.name, second.name) != 0) {
        return false;
    }

    if (strcmp(first.channelName, second.channelName) != 0) {
        return false;
    }

    if (strcmp(first.clusterName, second.clusterName) != 0) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const EthControllerContainer& first, const EthControllerContainer& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (!Equals(first.macAddress, second.macAddress)) {
        return false;
    }

    if (!Equals(first.name, second.name)) {
        return false;
    }

    if (!Equals(first.channelName, second.channelName)) {
        return false;
    }

    if (!Equals(first.clusterName, second.clusterName)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const EthMessage& first, const EthMessage& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data, second.data, first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const EthMessageContainer& first, const EthMessageContainer& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data.data(), second.data.data(), first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const LinController& first, const LinController& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (first.type != second.type) {
        return false;
    }

    if (strcmp(first.name, second.name) != 0) {
        return false;
    }

    if (strcmp(first.channelName, second.channelName) != 0) {
        return false;
    }

    if (strcmp(first.clusterName, second.clusterName) != 0) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const LinControllerContainer& first, const LinControllerContainer& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (first.type != second.type) {
        return false;
    }

    if (!Equals(first.name, second.name)) {
        return false;
    }

    if (!Equals(first.channelName, second.channelName)) {
        return false;
    }

    if (!Equals(first.clusterName, second.clusterName)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const LinMessage& first, const LinMessage& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.id != second.id) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data, second.data, first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const LinMessageContainer& first, const LinMessageContainer& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.id != second.id) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data.data(), second.data.data(), first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const FrController& first, const FrController& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (strcmp(first.name, second.name) != 0) {
        return false;
    }

    if (strcmp(first.channelName, second.channelName) != 0) {
        return false;
    }

    if (strcmp(first.clusterName, second.clusterName) != 0) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const FrControllerContainer& first, const FrControllerContainer& second) {
    if (first.id != second.id) {
        return false;
    }

    if (first.queueSize != second.queueSize) {
        return false;
    }

    if (first.bitsPerSecond != second.bitsPerSecond) {
        return false;
    }

    if (!Equals(first.name, second.name)) {
        return false;
    }

    if (!Equals(first.channelName, second.channelName)) {
        return false;
    }

    if (!Equals(first.clusterName, second.clusterName)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const FrMessage& first, const FrMessage& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.id != second.id) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data, second.data, first.length)) {
        return false;
    }

    return true;
}

[[nodiscard]] bool operator==(const FrMessageContainer& first, const FrMessageContainer& second) {
    if (first.timestamp != second.timestamp) {
        return false;
    }

    if (first.controllerId != second.controllerId) {
        return false;
    }

    if (first.id != second.id) {
        return false;
    }

    if (first.flags != second.flags) {
        return false;
    }

    if (first.length != second.length) {
        return false;
    }

    if (!Equals(first.data.data(), second.data.data(), first.length)) {
        return false;
    }

    return true;
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

[[nodiscard]] CanController CanControllerContainer::Convert() const {
    CanController canController{};
    canController.id = id;
    canController.queueSize = queueSize;
    canController.bitsPerSecond = bitsPerSecond;
    canController.flexibleDataRateBitsPerSecond = flexibleDataRateBitsPerSecond;
    canController.name = name.c_str();
    canController.channelName = channelName.c_str();
    canController.clusterName = clusterName.c_str();
    return canController;
}

[[nodiscard]] EthController EthControllerContainer::Convert() const {
    EthController ethController{};
    ethController.id = id;
    ethController.queueSize = queueSize;
    ethController.bitsPerSecond = bitsPerSecond;
    ethController.macAddress = macAddress;
    ethController.name = name.c_str();
    ethController.channelName = channelName.c_str();
    ethController.clusterName = clusterName.c_str();
    return ethController;
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

[[nodiscard]] FrController FrControllerContainer::Convert() const {
    FrController controller{};
    controller.id = id;
    controller.queueSize = queueSize;
    controller.bitsPerSecond = bitsPerSecond;
    controller.name = name.c_str();
    controller.channelName = channelName.c_str();
    controller.clusterName = clusterName.c_str();
    return controller;
}

[[nodiscard]] std::vector<IoSignal> Convert(const std::vector<IoSignalContainer>& ioSignalContainers) {
    std::vector<IoSignal> ioSignals;
    ioSignals.reserve(ioSignalContainers.size());

    for (const auto& ioSignalContainer : ioSignalContainers) {
        ioSignals.push_back(ioSignalContainer.Convert());
    }

    return ioSignals;
}

[[nodiscard]] std::vector<CanController> Convert(const std::vector<CanControllerContainer>& canControllerContainers) {
    std::vector<CanController> controllers;
    controllers.reserve(canControllerContainers.size());

    for (const auto& canControllerContainer : canControllerContainers) {
        controllers.push_back(canControllerContainer.Convert());
    }

    return controllers;
}

[[nodiscard]] std::vector<EthController> Convert(const std::vector<EthControllerContainer>& ethControllerContainers) {
    std::vector<EthController> controllers;
    controllers.reserve(ethControllerContainers.size());

    for (const auto& ethControllerContainer : ethControllerContainers) {
        controllers.push_back(ethControllerContainer.Convert());
    }

    return controllers;
}

[[nodiscard]] std::vector<LinController> Convert(const std::vector<LinControllerContainer>& linControllerContainers) {
    std::vector<LinController> controllers;
    controllers.reserve(linControllerContainers.size());

    for (const auto& linControllerContainer : linControllerContainers) {
        controllers.push_back(linControllerContainer.Convert());
    }

    return controllers;
}

[[nodiscard]] std::vector<FrController> Convert(const std::vector<FrControllerContainer>& frControllerContainers) {
    std::vector<FrController> controllers;
    controllers.reserve(frControllerContainers.size());

    for (const auto& frControllerContainer : frControllerContainers) {
        controllers.push_back(frControllerContainer.Convert());
    }

    return controllers;
}

[[nodiscard]] Result CanMessage::Check() const {
    return CheckCanMessage(length);
}

[[nodiscard]] Result CanMessageContainer::Check() const {
    return CheckCanMessage(length);
}

[[nodiscard]] Result EthMessage::Check() const {
    return EthMessageCheck(length);
}

[[nodiscard]] Result EthMessageContainer::Check() const {
    return EthMessageCheck(length);
}

[[nodiscard]] Result LinMessage::Check() const {
    return LinMessageCheck(length);
}

[[nodiscard]] Result LinMessageContainer::Check() const {
    return LinMessageCheck(length);
}

[[nodiscard]] Result FrMessage::Check() const {
    return FrMessageCheck(length);
}

[[nodiscard]] Result FrMessageContainer::Check() const {
    return FrMessageCheck(length);
}

void CanMessage::WriteTo(CanMessageContainer& canMessageContainer) const {
    canMessageContainer.timestamp = timestamp;
    canMessageContainer.controllerId = controllerId;
    canMessageContainer.id = id;
    canMessageContainer.flags = flags;
    canMessageContainer.length = length;
    (void)memcpy(canMessageContainer.data.data(), data, length);
}

void CanMessageContainer::WriteTo(CanMessage& canMessage) const {
    canMessage.timestamp = timestamp;
    canMessage.controllerId = controllerId;
    canMessage.id = id;
    canMessage.flags = flags;
    canMessage.length = length;
    canMessage.data = data.data();
}

void EthMessage::WriteTo(EthMessageContainer& ethMessageContainer) const {
    ethMessageContainer.timestamp = timestamp;
    ethMessageContainer.controllerId = controllerId;
    ethMessageContainer.flags = flags;
    ethMessageContainer.length = length;
    (void)memcpy(ethMessageContainer.data.data(), data, length);
}

void EthMessageContainer::WriteTo(EthMessage& ethMessage) const {
    ethMessage.timestamp = timestamp;
    ethMessage.controllerId = controllerId;
    ethMessage.flags = flags;
    ethMessage.length = length;
    ethMessage.data = data.data();
}

void LinMessage::WriteTo(LinMessageContainer& linMessageContainer) const {
    linMessageContainer.timestamp = timestamp;
    linMessageContainer.controllerId = controllerId;
    linMessageContainer.id = id;
    linMessageContainer.flags = flags;
    linMessageContainer.length = length;
    (void)memcpy(linMessageContainer.data.data(), data, length);
}

void LinMessageContainer::WriteTo(LinMessage& linMessage) const {
    linMessage.timestamp = timestamp;
    linMessage.controllerId = controllerId;
    linMessage.id = id;
    linMessage.flags = flags;
    linMessage.length = length;
    linMessage.data = data.data();
}

void FrMessage::WriteTo(FrMessageContainer& frMessageContainer) const {
    frMessageContainer.timestamp = timestamp;
    frMessageContainer.controllerId = controllerId;
    frMessageContainer.id = id;
    frMessageContainer.flags = flags;
    frMessageContainer.length = length;
    (void)memcpy(frMessageContainer.data.data(), data, length);
}

void FrMessageContainer::WriteTo(FrMessage& frMessage) const {
    frMessage.timestamp = timestamp;
    frMessage.controllerId = controllerId;
    frMessage.id = id;
    frMessage.flags = flags;
    frMessage.length = length;
    frMessage.data = data.data();
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

}  // namespace DsVeosCoSim
