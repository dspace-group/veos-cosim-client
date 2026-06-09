// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <array>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

#include <BusExchange.hpp>
#include <Channel.hpp>
#include <CoSimTypes.hpp>
#include <Protocol.hpp>
#include <Socket.hpp>

#include "Helper.hpp"

#ifdef _WIN32

#include <OsUtilities.hpp>

#endif

[[maybe_unused]] constexpr uint32_t DefaultTimeoutInMilliseconds = 1000;

#define AssertOk(result) ASSERT_EQ(result, DsVeosCoSim::CreateOk())
#define AssertError(result) ASSERT_EQ(result, DsVeosCoSim::CreateError())
#define AssertTimeout(result) ASSERT_EQ(result, DsVeosCoSim::CreateTimeout())
#define AssertNotConnected(result) ASSERT_EQ(result, DsVeosCoSim::CreateNotConnected())
#define AssertInvalidArgument(result) ASSERT_EQ(result, DsVeosCoSim::CreateInvalidArgument())
#define AssertFull(result) ASSERT_EQ(result, DsVeosCoSim::CreateFull())
#define AssertEmpty(result) ASSERT_EQ(result, DsVeosCoSim::CreateEmpty())

[[nodiscard]] DsVeosCoSim::Result StartUp();

[[nodiscard]] constexpr const char* GetLoopBackAddress(DsVeosCoSim::AddressFamily addressFamily) noexcept {
    if (addressFamily == DsVeosCoSim::AddressFamily::Ipv4) {
        return "127.0.0.1";
    }

    return "::1";
}

template <typename T, typename = std::void_t<T>>
struct HasDirectSendReceive : std::false_type {};

template <typename T>
struct HasDirectSendReceive<T,
                            std::void_t<decltype(std::declval<T&>().Send(static_cast<const void*>(nullptr), std::declval<size_t>())),
                                        decltype(std::declval<T&>().Receive(static_cast<void*>(nullptr), std::declval<size_t>(), std::declval<size_t&>()))>>
    : std::true_type {};

// ReceiveComplete and socket-like test helpers are templates so they work
// with both SocketClient and ShmPipeClient without duplication.
template <typename TClient, std::enable_if_t<HasDirectSendReceive<TClient>::value, int> = 0>
[[nodiscard]] DsVeosCoSim::Result ReceiveComplete(TClient& client, void* buffer, size_t length) {
    auto* bufferPointer = static_cast<uint8_t*>(buffer);
    while (length > 0) {
        size_t receivedSize{};
        CheckResult(client.Receive(bufferPointer, length, receivedSize));
        length -= receivedSize;
        bufferPointer += receivedSize;
    }

    return DsVeosCoSim::CreateOk();
}

template <typename TClient, std::enable_if_t<HasDirectSendReceive<TClient>::value, int> = 0>
void TestSendAfterDisconnect(TClient& client) {
    client.Disconnect();
    size_t sendValue = GenerateSizeT();
    DsVeosCoSim::Result result = client.Send(&sendValue, sizeof(sendValue));
    AssertNotConnected(result);
}

template <typename TClient, std::enable_if_t<HasDirectSendReceive<TClient>::value, int> = 0>
void TestReceiveAfterDisconnect(TClient& client) {
    client.Disconnect();
    size_t receiveValue{};
    size_t receivedSize{};
    DsVeosCoSim::Result result = client.Receive(&receiveValue, sizeof(receiveValue), receivedSize);
    AssertNotConnected(result);
}

template <typename TClient, std::enable_if_t<HasDirectSendReceive<TClient>::value, int> = 0>
void TestReceiveAfterDisconnectOnRemoteClient(TClient& client1, TClient& client2) {
    client1.Disconnect();
    size_t receiveValue{};
    size_t receivedSize{};
    DsVeosCoSim::Result result = client2.Receive(&receiveValue, sizeof(receiveValue), receivedSize);
    AssertNotConnected(result);
}

template <typename TClient, std::enable_if_t<HasDirectSendReceive<TClient>::value, int> = 0>
void TestSendAndReceive(TClient& client1, TClient& client2) {
    size_t sendValue = GenerateSizeT();
    size_t receiveValue = 0;
    DsVeosCoSim::Result sendResult = client1.Send(&sendValue, sizeof(sendValue));
    DsVeosCoSim::Result receiveResult = ReceiveComplete(client2, &receiveValue, sizeof(receiveValue));
    AssertOk(sendResult);
    AssertOk(receiveResult);
    ASSERT_EQ(sendValue, receiveValue);
}

template <typename TClient, std::enable_if_t<HasDirectSendReceive<TClient>::value, int> = 0>
void TestManyElements(TClient& client1, TClient& client2) {
    constexpr size_t count = 0x100;
    std::thread thread([&] {
        for (size_t i = 0; i < count; i++) {
            size_t receiveValue{};
            AssertOk(ReceiveComplete(client2, &receiveValue, sizeof(receiveValue)));
            ASSERT_EQ(i, receiveValue);
        }
    });

    for (size_t i = 0; i < count; i++) {
        AssertOk(client1.Send(&i, sizeof(i)));
    }

    thread.join();
}

template <typename TClient, std::enable_if_t<HasDirectSendReceive<TClient>::value, int> = 0>
void TestBigElement(TClient& client1, TClient& client2) {
    constexpr size_t count = 0x1000;
    std::thread thread([&] {
        auto receiveArray = std::make_unique<std::array<size_t, count>>();
        AssertOk(ReceiveComplete(client2, receiveArray->data(), receiveArray->size() * sizeof(size_t)));
        for (size_t i = 0; i < receiveArray->size(); i++) {
            ASSERT_EQ(i, (*receiveArray)[i]);
        }
    });

    auto sendArray = std::make_unique<std::array<size_t, count>>();
    for (size_t i = 0; i < sendArray->size(); i++) {
        (*sendArray)[i] = i;
    }

    AssertOk(client1.Send(sendArray->data(), sendArray->size() * sizeof(size_t)));
    thread.join();
}

template <typename TClient, std::enable_if_t<HasDirectSendReceive<TClient>::value, int> = 0>
void TestPingPong(TClient& client1, TClient& client2) {
    constexpr size_t count = 100;
    for (size_t i = 0; i < count; i++) {
        TClient* sendClient = &client1;
        TClient* receiveClient = &client2;
        if (i % 2 == 1) {
            std::swap(sendClient, receiveClient);
        }

        size_t sendValue = GenerateSizeT();
        AssertOk(sendClient->Send(&sendValue, sizeof(sendValue)));
        size_t receiveValue{};
        AssertOk(ReceiveComplete(*receiveClient, &receiveValue, sizeof(receiveValue)));
        ASSERT_EQ(sendValue, receiveValue);
    }
}

#ifdef _WIN32

void TestSendAfterDisconnectOnRemoteClient(const DsVeosCoSim::ShmPipeClient& client1, DsVeosCoSim::ShmPipeClient& client2);

#endif

[[nodiscard]] DsVeosCoSim::CoSimType GetCounterPart(DsVeosCoSim::CoSimType coSimType);
[[nodiscard]] std::string GetCounterPart(const std::string& name, DsVeosCoSim::ConnectionKind connectionKind);

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

void TestWriteUInt16ToChannel(const std::unique_ptr<DsVeosCoSim::Channel>& writeChannel);
void TestWriteUInt32ToChannel(const std::unique_ptr<DsVeosCoSim::Channel>& writeChannel);
void TestWriteUInt64ToChannel(const std::unique_ptr<DsVeosCoSim::Channel>& writeChannel);
void TestWriteBufferToChannel(const std::unique_ptr<DsVeosCoSim::Channel>& writeChannel);

void TestReadUInt16FromChannel(const std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, const std::unique_ptr<DsVeosCoSim::Channel>& readChannel);
void TestReadUInt32FromChannel(const std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, const std::unique_ptr<DsVeosCoSim::Channel>& readChannel);
void TestReadUInt64FromChannel(const std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, const std::unique_ptr<DsVeosCoSim::Channel>& readChannel);
void TestReadBufferFromChannel(const std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, const std::unique_ptr<DsVeosCoSim::Channel>& readChannel);

void TestPingPong(const std::unique_ptr<DsVeosCoSim::Channel>& firstChannel, const std::unique_ptr<DsVeosCoSim::Channel>& secondChannel);

void TestSendTwoFramesAtOnce(const std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, const std::unique_ptr<DsVeosCoSim::Channel>& readChannel);

void TestStream(const std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, const std::unique_ptr<DsVeosCoSim::Channel>& readChannel);

void TestBigElement(const std::unique_ptr<DsVeosCoSim::Channel>& writeChannel, const std::unique_ptr<DsVeosCoSim::Channel>& readChannel);

namespace DsVeosCoSim {

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
