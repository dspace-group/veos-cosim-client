// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <fmt/format.h>

#include "BusBuffer.h"  // IWYU pragma: keep
#include "DsVeosCoSim/CoSimTypes.h"
#include "Socket.h"

namespace DsVeosCoSim {

[[maybe_unused]] constexpr int32_t F6 = -64;
[[maybe_unused]] constexpr int32_t F8 = -66;
[[maybe_unused]] constexpr int32_t F9 = -67;
[[maybe_unused]] constexpr int32_t F10 = -68;
[[maybe_unused]] constexpr int32_t F11 = -69;
[[maybe_unused]] constexpr int32_t F12 = -70;

[[maybe_unused]] constexpr uint32_t DefaultTimeout = 1000;

#define MustBeOk(result)                                             \
    do {                                                             \
        Result _result_ = (result);                                  \
        if (!IsOk(_result_)) {                                       \
            LogError("Expected Ok but was {}.", ToString(_result_)); \
            exit(1);                                                 \
        }                                                            \
    } while (0)

#define MustBeDisconnected(result)                                             \
    do {                                                                       \
        Result _result_ = (result);                                            \
        if (_result_ != Result::Disconnected) {                                \
            LogError("Expected Disconnected but was {}.", ToString(_result_)); \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

#define MustBeTrue(result)                            \
    do {                                              \
        if (!(result)) {                              \
            LogError("Expected true but was false."); \
            exit(1);                                  \
        }                                             \
    } while (0)

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

template <typename... T>
void LogError(fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    OnLogCallback(DsVeosCoSim::Severity::Error, fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogWarning(fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    OnLogCallback(DsVeosCoSim::Severity::Warning, fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogInfo(fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    OnLogCallback(DsVeosCoSim::Severity::Info, fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogTrace(fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    OnLogCallback(DsVeosCoSim::Severity::Trace, fmt::vformat(format, fmt::make_format_args(args...)));
}

void LogError(const std::string& message);
void LogWarning(const std::string& message);
void LogInfo(const std::string& message);
void LogTrace(const std::string& message);

void InitializeOutput();

void OnLogCallback(Severity severity, const std::string& message);

void ClearLastMessage();
[[nodiscard]] std::string GetLastMessage();

[[nodiscard]] int32_t GetChar();

void SetEnvVariable(const std::string& name, const std::string& value);

[[nodiscard]] Result StartUp();

[[nodiscard]] const char* GetLoopBackAddress(AddressFamily addressFamily);

[[nodiscard]] Result ReceiveComplete(const Socket& socket, void* buffer, size_t length);

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
                                     const std::vector<CanController>& controllers,
                                     IProtocol& protocol,
                                     std::unique_ptr<BusBuffer>& busBuffer);

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
                                     const std::vector<EthController>& controllers,
                                     IProtocol& protocol,
                                     std::unique_ptr<BusBuffer>& busBuffer);

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
                                     const std::vector<LinController>& controllers,
                                     IProtocol& protocol,
                                     std::unique_ptr<BusBuffer>& busBuffer);

[[nodiscard]] Result CreateBusBuffer(CoSimType coSimType,
                                     ConnectionKind connectionKind,
                                     const std::string& name,
                                     const std::vector<FrController>& controllers,
                                     IProtocol& protocol,
                                     std::unique_ptr<BusBuffer>& busBuffer);

[[nodiscard]] uint8_t GenerateU8();
[[nodiscard]] uint16_t GenerateU16();
[[nodiscard]] uint32_t GenerateU32();
[[nodiscard]] uint64_t GenerateU64();

[[nodiscard]] int64_t GenerateI64();

void FillWithRandomData(uint8_t* data, size_t length);

template <typename T>
[[nodiscard]] T GenerateRandom(T min, T max) {
    uint32_t diff = static_cast<uint32_t>(max) + 1 - static_cast<uint32_t>(min);
    return static_cast<T>(static_cast<uint32_t>(min) + (GenerateU32() % diff));
}

[[nodiscard]] std::string GenerateString(const std::string& prefix);
[[nodiscard]] SimulationTime GenerateSimulationTime();
[[nodiscard]] BusMessageId GenerateBusMessageId(uint32_t min, uint32_t max);
[[nodiscard]] BusControllerId GenerateBusControllerId();
[[nodiscard]] IoSignalId GenerateIoSignalId();
[[nodiscard]] std::vector<uint8_t> GenerateBytes(size_t length);

[[nodiscard]] IoSignalContainer CreateSignal();
[[nodiscard]] IoSignalContainer CreateSignal(DataType dataType);
[[nodiscard]] IoSignalContainer CreateSignal(DataType dataType, SizeKind sizeKind);

[[nodiscard]] std::vector<uint8_t> GenerateIoData(const IoSignalContainer& signal);
[[nodiscard]] std::vector<uint8_t> CreateZeroedIoData(const IoSignalContainer& signal);

void FillWithRandom(CanControllerContainer& controller);
void FillWithRandom(EthControllerContainer& controller);
void FillWithRandom(LinControllerContainer& controller);
void FillWithRandom(FrControllerContainer& controller);

void FillWithRandom(CanMessageContainer& message, BusControllerId controllerId);
void FillWithRandom(EthMessageContainer& message, BusControllerId controllerId);
void FillWithRandom(LinMessageContainer& message, BusControllerId controllerId);
void FillWithRandom(FrMessageContainer& message, BusControllerId controllerId);

[[nodiscard]] std::vector<IoSignalContainer> CreateSignals(size_t count);

[[nodiscard]] std::vector<CanControllerContainer> CreateCanControllers(size_t count);
[[nodiscard]] std::vector<EthControllerContainer> CreateEthControllers(size_t count);
[[nodiscard]] std::vector<LinControllerContainer> CreateLinControllers(size_t count);
[[nodiscard]] std::vector<FrControllerContainer> CreateFrControllers(size_t count);

}  // namespace DsVeosCoSim
