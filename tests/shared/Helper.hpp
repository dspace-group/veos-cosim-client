// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>

#include "BusExchange.hpp"
#include "CoSimTypes.hpp"
#include "Logger.hpp"
#include "Protocol.hpp"
#include "Result.hpp"  // IWYU pragma: keep
#include "Socket.hpp"

#ifdef _WIN32

#include "OsUtilities.hpp"

#endif

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

void InitializeOutput();
void SetMinimalSeverity(DsVeosCoSim::Severity severity);

void MustBeOk(const DsVeosCoSim::Result& result);
void MustBeNotConnected(const DsVeosCoSim::Result& result);

void LogIoData(std::string_view ioDataStr);
void LogCanMessage(std::string_view canMessageStr);
void LogEthMessage(std::string_view ethMessageStr);
void LogLinMessage(std::string_view linMessageStr);
void LogFrMessage(std::string_view linMessageStr);

[[nodiscard]] int32_t GetChar();

void SetEnvVariable(const std::string& name, const std::string& value);

[[nodiscard]] DsVeosCoSim::Result StartUp();

[[nodiscard]] constexpr const char* GetLoopBackAddress(DsVeosCoSim::AddressFamily addressFamily) noexcept {
    if (addressFamily == DsVeosCoSim::AddressFamily::Ipv4) {
        return "127.0.0.1";
    }

    return "::1";
}

[[nodiscard]] DsVeosCoSim::Result ReceiveComplete(const DsVeosCoSim::SocketClient& client, void* buffer, size_t length);

#ifdef _WIN32

[[nodiscard]] DsVeosCoSim::Result ReceiveComplete(DsVeosCoSim::ShmPipeClient& client, void* buffer, size_t length);

#endif

[[nodiscard]] DsVeosCoSim::Result CreateBusExchange(DsVeosCoSim::CoSimType coSimType,
                                                    DsVeosCoSim::ConnectionKind connectionKind,
                                                    std::string_view name,
                                                    const std::vector<DsVeosCoSim::CanController>& controllers,
                                                    DsVeosCoSim::IProtocol& protocol,
                                                    std::unique_ptr<DsVeosCoSim::BusExchange>& busExchange);

[[nodiscard]] DsVeosCoSim::Result CreateBusExchange(DsVeosCoSim::CoSimType coSimType,
                                                    DsVeosCoSim::ConnectionKind connectionKind,
                                                    std::string_view name,
                                                    const std::vector<DsVeosCoSim::EthController>& controllers,
                                                    DsVeosCoSim::IProtocol& protocol,
                                                    std::unique_ptr<DsVeosCoSim::BusExchange>& busExchange);

[[nodiscard]] DsVeosCoSim::Result CreateBusExchange(DsVeosCoSim::CoSimType coSimType,
                                                    DsVeosCoSim::ConnectionKind connectionKind,
                                                    std::string_view name,
                                                    const std::vector<DsVeosCoSim::LinController>& controllers,
                                                    DsVeosCoSim::IProtocol& protocol,
                                                    std::unique_ptr<DsVeosCoSim::BusExchange>& busExchange);

[[nodiscard]] DsVeosCoSim::Result CreateBusExchange(DsVeosCoSim::CoSimType coSimType,
                                                    DsVeosCoSim::ConnectionKind connectionKind,
                                                    std::string_view name,
                                                    const std::vector<DsVeosCoSim::FrController>& controllers,
                                                    DsVeosCoSim::IProtocol& protocol,
                                                    std::unique_ptr<DsVeosCoSim::BusExchange>& busExchange);

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

[[nodiscard]] std::string GenerateString(std::string_view prefix);
[[nodiscard]] DsVeosCoSim::SimulationTime GenerateSimulationTime();
[[nodiscard]] DsVeosCoSim::BusMessageId GenerateBusMessageId(uint32_t min, uint32_t max);
[[nodiscard]] DsVeosCoSim::BusControllerId GenerateBusControllerId();
[[nodiscard]] DsVeosCoSim::IoSignalId GenerateIoSignalId();
[[nodiscard]] std::vector<uint8_t> GenerateBytes(size_t length);

[[nodiscard]] DsVeosCoSim::IoSignalContainer CreateSignal();
[[nodiscard]] DsVeosCoSim::IoSignalContainer CreateSignal(DsVeosCoSim::DataType dataType);
[[nodiscard]] DsVeosCoSim::IoSignalContainer CreateSignal(DsVeosCoSim::DataType dataType, DsVeosCoSim::SizeKind sizeKind);

[[nodiscard]] std::vector<uint8_t> GenerateIoData(const DsVeosCoSim::IoSignalContainer& signal);
[[nodiscard]] std::vector<uint8_t> CreateZeroedIoData(const DsVeosCoSim::IoSignalContainer& signal);

void FillWithRandom(DsVeosCoSim::CanControllerContainer& controller);
void FillWithRandom(DsVeosCoSim::EthControllerContainer& controller);
void FillWithRandom(DsVeosCoSim::LinControllerContainer& controller);
void FillWithRandom(DsVeosCoSim::FrControllerContainer& controller);

void FillWithRandom(DsVeosCoSim::CanMessageContainer& message, DsVeosCoSim::BusControllerId controllerId);
void FillWithRandom(DsVeosCoSim::EthMessageContainer& message, DsVeosCoSim::BusControllerId controllerId);
void FillWithRandom(DsVeosCoSim::LinMessageContainer& message, DsVeosCoSim::BusControllerId controllerId);
void FillWithRandom(DsVeosCoSim::FrMessageContainer& message, DsVeosCoSim::BusControllerId controllerId);

[[nodiscard]] std::vector<DsVeosCoSim::IoSignalContainer> CreateSignals(size_t count);
[[nodiscard]] std::vector<DsVeosCoSim::CanControllerContainer> CreateCanControllers(size_t count);
[[nodiscard]] std::vector<DsVeosCoSim::EthControllerContainer> CreateEthControllers(size_t count);
[[nodiscard]] std::vector<DsVeosCoSim::LinControllerContainer> CreateLinControllers(size_t count);
[[nodiscard]] std::vector<DsVeosCoSim::FrControllerContainer> CreateFrControllers(size_t count);

std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::SimulationTime simulationTime);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::Result result);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::CoSimType coSimType);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::ConnectionKind connectionKind);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::Command command);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::Severity severity);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::TerminateReason terminateReason);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::ConnectionState connectionState);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::SimulationState simulationState);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::Mode mode);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::IoSignalId ioSignalId);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::DataType dataType);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::SizeKind sizeKind);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::BusControllerId busControllerId);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::BusMessageId busMessageId);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::LinControllerType linControllerType);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::CanMessageFlags canMessageFlags);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::EthMessageFlags ethMessageFlags);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::LinMessageFlags linMessageFlags);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::FrMessageFlags frMessageFlags);
std::ostream& operator<<(std::ostream& stream, DsVeosCoSim::FrameKind frameKind);

std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::IoSignal& ioSignal);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::IoSignalContainer& ioSignalContainer);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::CanController& canController);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::CanControllerContainer& canControllerContainer);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::CanMessage& canMessage);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::CanMessageContainer& canMessageContainer);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::EthController& ethController);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::EthControllerContainer& ethControllerContainer);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::EthMessage& ethMessage);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::EthMessageContainer& ethMessageContainer);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::LinController& linController);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::LinControllerContainer& frControllerContainer);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::LinMessage& linMessage);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::LinMessageContainer& linMessageContainer);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::FrController& frController);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::FrControllerContainer& frControllerContainer);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::FrMessage& frMessage);
std::ostream& operator<<(std::ostream& stream, const DsVeosCoSim::FrMessageContainer& frMessageContainer);

std::ostream& operator<<(std::ostream& stream, const std::vector<DsVeosCoSim::IoSignalContainer>& ioSignalContainers);
std::ostream& operator<<(std::ostream& stream, const std::vector<DsVeosCoSim::CanControllerContainer>& canControllerContainers);
std::ostream& operator<<(std::ostream& stream, const std::vector<DsVeosCoSim::EthControllerContainer>& ethControllerContainers);
std::ostream& operator<<(std::ostream& stream, const std::vector<DsVeosCoSim::LinControllerContainer>& linControllerContainers);
std::ostream& operator<<(std::ostream& stream, const std::vector<DsVeosCoSim::FrControllerContainer>& frControllerContainers);
