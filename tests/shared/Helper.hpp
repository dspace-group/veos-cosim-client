// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "BusBuffer.hpp"
#include "CoSimTypes.hpp"
#include "Error.hpp"  // IWYU pragma: keep
#include "Logger.hpp"
#include "Protocol.hpp"
#include "Socket.hpp"

#ifdef _WIN32

#include "OsUtilities.hpp"

#endif

namespace DsVeosCoSim {

[[maybe_unused]] constexpr int32_t F6 = -64;
[[maybe_unused]] constexpr int32_t F8 = -66;
[[maybe_unused]] constexpr int32_t F9 = -67;
[[maybe_unused]] constexpr int32_t F10 = -68;
[[maybe_unused]] constexpr int32_t F11 = -69;
[[maybe_unused]] constexpr int32_t F12 = -70;

[[maybe_unused]] constexpr uint32_t DefaultTimeout = 1000;

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

void LogError(const std::string& message);
void LogWarning(const std::string& message);
void LogInfo(const std::string& message);
void LogTrace(const std::string& message);

template <typename... T>
void LogError(fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    LogError(fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogWarning(fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    LogWarning(fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogInfo(fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    LogInfo(fmt::vformat(format, fmt::make_format_args(args...)));
}

template <typename... T>
void LogTrace(fmt::format_string<T...> format, T&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    LogTrace(fmt::vformat(format, fmt::make_format_args(args...)));
}

void InitializeOutput();

void MustBeOk(const Result& result);

void OnLogCallback(Severity severity, std::string_view message);

void ClearLastMessage();
[[nodiscard]] std::string GetLastMessage();

[[nodiscard]] int32_t GetChar();

void SetEnvVariable(const std::string& name, const std::string& value);

[[nodiscard]] Result StartUp();

[[nodiscard]] const char* GetLoopBackAddress(AddressFamily addressFamily);

[[nodiscard]] Result ReceiveComplete(const SocketClient& client, void* buffer, size_t length);

#ifdef _WIN32

[[nodiscard]] Result ReceiveComplete(ShmPipeClient& client, void* buffer, size_t length);

#endif

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
[[nodiscard]] size_t GenerateSizeT();

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

std::ostream& operator<<(std::ostream& stream, SimulationTime simulationTime);
std::ostream& operator<<(std::ostream& stream, Result result);
std::ostream& operator<<(std::ostream& stream, CoSimType coSimType);
std::ostream& operator<<(std::ostream& stream, ConnectionKind connectionKind);
std::ostream& operator<<(std::ostream& stream, Command command);
std::ostream& operator<<(std::ostream& stream, Severity severity);
std::ostream& operator<<(std::ostream& stream, TerminateReason terminateReason);
std::ostream& operator<<(std::ostream& stream, ConnectionState connectionState);
std::ostream& operator<<(std::ostream& stream, SimulationState simulationState);
std::ostream& operator<<(std::ostream& stream, Mode mode);
std::ostream& operator<<(std::ostream& stream, IoSignalId ioSignalId);
std::ostream& operator<<(std::ostream& stream, DataType dataType);
std::ostream& operator<<(std::ostream& stream, SizeKind sizeKind);
std::ostream& operator<<(std::ostream& stream, BusControllerId busControllerId);
std::ostream& operator<<(std::ostream& stream, BusMessageId busMessageId);
std::ostream& operator<<(std::ostream& stream, LinControllerType linControllerType);
std::ostream& operator<<(std::ostream& stream, CanMessageFlags canMessageFlags);
std::ostream& operator<<(std::ostream& stream, EthMessageFlags ethMessageFlags);
std::ostream& operator<<(std::ostream& stream, LinMessageFlags linMessageFlags);
std::ostream& operator<<(std::ostream& stream, FrMessageFlags frMessageFlags);
std::ostream& operator<<(std::ostream& stream, FrameKind frameKind);

std::ostream& operator<<(std::ostream& stream, const IoSignal& ioSignal);
std::ostream& operator<<(std::ostream& stream, const IoSignalContainer& ioSignalContainer);
std::ostream& operator<<(std::ostream& stream, const CanController& canController);
std::ostream& operator<<(std::ostream& stream, const CanControllerContainer& canControllerContainer);
std::ostream& operator<<(std::ostream& stream, const CanMessage& canMessage);
std::ostream& operator<<(std::ostream& stream, const CanMessageContainer& canMessageContainer);
std::ostream& operator<<(std::ostream& stream, const EthController& ethController);
std::ostream& operator<<(std::ostream& stream, const EthControllerContainer& ethControllerContainer);
std::ostream& operator<<(std::ostream& stream, const EthMessage& ethMessage);
std::ostream& operator<<(std::ostream& stream, const EthMessageContainer& ethMessageContainer);
std::ostream& operator<<(std::ostream& stream, const LinController& linController);
std::ostream& operator<<(std::ostream& stream, const LinControllerContainer& frControllerContainer);
std::ostream& operator<<(std::ostream& stream, const LinMessage& linMessage);
std::ostream& operator<<(std::ostream& stream, const LinMessageContainer& linMessageContainer);
std::ostream& operator<<(std::ostream& stream, const FrController& frController);
std::ostream& operator<<(std::ostream& stream, const FrControllerContainer& frControllerContainer);
std::ostream& operator<<(std::ostream& stream, const FrMessage& frMessage);
std::ostream& operator<<(std::ostream& stream, const FrMessageContainer& frMessageContainer);

std::ostream& operator<<(std::ostream& stream, const std::vector<IoSignalContainer>& ioSignalContainers);
std::ostream& operator<<(std::ostream& stream, const std::vector<CanControllerContainer>& canControllerContainers);
std::ostream& operator<<(std::ostream& stream, const std::vector<EthControllerContainer>& ethControllerContainers);
std::ostream& operator<<(std::ostream& stream, const std::vector<LinControllerContainer>& linControllerContainers);
std::ostream& operator<<(std::ostream& stream, const std::vector<FrControllerContainer>& frControllerContainers);

}  // namespace DsVeosCoSim
