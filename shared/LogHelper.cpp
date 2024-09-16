// Copyright dSPACE GmbH. All rights reserved.

#include "LogHelper.h"

#include <fmt/color.h>
#include <iomanip>
#include <sstream>
#include "Logger.h"

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace DsVeosCoSim;

namespace {

std::string g_lastMessage;

fmt::text_style red = fg(fmt::color::red);
fmt::text_style cyan = fg(fmt::color::cyan);
fmt::text_style green = fg(fmt::color::lime);
fmt::text_style yellow = fg(fmt::color::yellow);
fmt::text_style white = fg(fmt::color::white);
fmt::text_style gray = fg(fmt::color::light_gray);
fmt::text_style blue = fg(fmt::color::dodger_blue);
fmt::text_style violet = fg(fmt::color::fuchsia);

std::string DataToString(const uint8_t* data, uint32_t dataLength, char separator = 0) {
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

std::string DataTypeValueToString(const void* value, uint32_t index, DsVeosCoSim_DataType dataType) {
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
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            return "";
    }
}

std::string ValueToString(const void* value, uint32_t length, DsVeosCoSim_DataType dataType) {
    std::ostringstream oss;
    for (uint32_t i = 0; i < length; i++) {
        if (i > 0) {
            oss << " ";
        }

        oss << DataTypeValueToString(value, i, dataType);
    }

    return oss.str();
}

}  // namespace

void InitializeOutput() {
#if _WIN32
    (void)::SetConsoleOutputCP(CP_UTF8);
    (void)::setvbuf(stdout, nullptr, _IONBF, 0);

    HANDLE console = ::GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwMode = 0;
    if (::GetConsoleMode(console, &dwMode) != 0) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        ::SetConsoleMode(console, dwMode);
    }
#endif

    SetLogCallback(OnLogCallback);
}

void OnLogCallback(Severity severity, std::string_view message) {
    g_lastMessage = message;
    switch (severity) {
        case Severity::Error:
            print(red, "{}\n", message);
            break;
        case Severity::Warning:
            print(yellow, "{}\n", message);
            break;
        case Severity::Info:
            print(white, "{}\n", message);
            break;
        case Severity::Trace:
            print(gray, "{}\n", message);
            break;
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            break;
    }
}

void LogIoSignal(const DsVeosCoSim_IoSignal& ioSignal) {
    LogTrace("  {} (id: {}, data type: {}, size kind: {}, length: {})",
             ioSignal.name,
             ioSignal.id,
             ToString(ioSignal.dataType),
             ToString(ioSignal.sizeKind),
             ioSignal.length);
}

void LogIoData(SimulationTime simulationTime,
               const DsVeosCoSim_IoSignal& ioSignal,
               uint32_t length,
               const void* value) {
    print(violet,
          "{},{},{},{}\n",
          SimulationTimeToSeconds(simulationTime),
          ioSignal.name,
          length,
          ValueToString(value, length, ioSignal.dataType));
}

void LogCanController(const DsVeosCoSim_CanController& controller) {
    LogTrace("  {} (id: {}, Size: {}, BitsPerSecond: {}, CAN FD BitsPerSecond: {}, Channel: {}, Cluster: {})",
             controller.name,
             controller.id,
             controller.queueSize,
             controller.bitsPerSecond,
             controller.flexibleDataRateBitsPerSecond,
             controller.channelName,
             controller.clusterName);
}

void LogEthController(const DsVeosCoSim_EthController& controller) {
    LogTrace("  {} (id: {}, Size: {}, BitsPerSecond: {}, MAC address: {}, Channel: {}, Cluster: {})",
             controller.name,
             controller.id,
             controller.queueSize,
             controller.bitsPerSecond,
             DataToString(controller.macAddress, EthAddressLength, ':'),
             controller.channelName,
             controller.clusterName);
}

void LogLinController(const DsVeosCoSim_LinController& controller) {
    LogTrace("  {} (id: {}, Size: {}, BitsPerSecond: {}, Type: {}, Channel: {}, Cluster: {})",
             controller.name,
             controller.id,
             controller.queueSize,
             controller.bitsPerSecond,
             ToString(controller.type),
             controller.channelName,
             controller.clusterName);
}

void LogCanMessage(SimulationTime simulationTime,
                   const DsVeosCoSim_CanController& controller,
                   const DsVeosCoSim_CanMessage& message) {
    print(blue,
          "{},{},{},{},{},CAN,{}\n",
          SimulationTimeToSeconds(simulationTime),
          controller.name,
          message.id,
          message.length,
          DataToString(message.data, message.length, '-'),
          CanMessageFlagsToString(message.flags));
}

void LogEthMessage(SimulationTime simulationTime,
                   const DsVeosCoSim_EthController& controller,
                   const DsVeosCoSim_EthMessage& message) {
    const uint8_t* data = message.data;
    const uint32_t length = message.length;

    if (length >= 14) {
        const std::string macAddress1 = DataToString(message.data, 6, ':');
        const std::string macAddress2 = DataToString(message.data + 6, 6, ':');
        const std::string ethernetType = DataToString(message.data + 12, 2);

        print(cyan,
              "{},{},{}-{},{},{},ETH,{},{}\n",
              SimulationTimeToSeconds(simulationTime),
              controller.name,
              macAddress2,
              macAddress1,
              length - 14,
              DataToString(data + 14, length - 14, '-'),
              ethernetType,
              EthMessageFlagsToString(message.flags));
    } else {
        print(cyan,
              "{},{},{},{},ETH,{}\n",
              SimulationTimeToSeconds(simulationTime),
              controller.name,
              length,
              DataToString(data, length, '-'),
              EthMessageFlagsToString(message.flags));
    }
}

void LogLinMessage(SimulationTime simulationTime,
                   const DsVeosCoSim_LinController& controller,
                   const DsVeosCoSim_LinMessage& message) {
    print(green,
          "{},{},{},{},{},LIN,{}\n",
          SimulationTimeToSeconds(simulationTime),
          controller.name,
          message.id,
          message.length,
          DataToString(message.data, message.length, '-'),
          LinMessageFlagsToString(message.flags));
}

void ClearLastMessage() {
    g_lastMessage = "";
}

std::string GetLastMessage() {
    return g_lastMessage;
}
