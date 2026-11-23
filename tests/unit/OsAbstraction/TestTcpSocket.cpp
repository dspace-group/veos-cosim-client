// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <future>
#include <ostream>
#include <string>
#include <thread>
#include <vector>

#include "CheckedAssertions.hpp"
#include "Result.hpp"
#include "Socket.hpp"
#include "TestHelper.hpp"

using namespace std::chrono_literals;
using namespace DsVeosCoSim;

namespace {

struct TcpSocketParam {
    AddressFamily addressFamily{};
    std::string listenAddress;
    std::string access;
};

[[maybe_unused]] void PrintTo(const TcpSocketParam& param, std::ostream* os)
{
    std::string name = fmt::format("{}_{}", param.addressFamily, param.access);
    *os << name;
}

[[nodiscard]] std::vector<TcpSocketParam> GetTcpSocketTestParameters()
{
    std::vector<TcpSocketParam> values;

    if (IsIpv4SocketSupported()) {
        values.push_back(TcpSocketParam{AddressFamily::Ipv4, "0.0.0.0", "Any"});
        values.push_back(TcpSocketParam{AddressFamily::Ipv4, "127.0.0.1", "Loopback"});
    }

    if (IsIpv6SocketSupported()) {
        values.push_back(TcpSocketParam{AddressFamily::Ipv6, "::", "Any"});
        values.push_back(TcpSocketParam{AddressFamily::Ipv6, "::1", "Loopback"});
    }

    return values;
}

void EstablishConnection(const TcpSocketParam& param, SocketClient& connectClient, SocketClient& acceptClient)
{
    SocketListener listener;
    AssertOk(SocketListener::Create(param.listenAddress, 0, listener));

    uint16_t port{};
    AssertOk(listener.GetLocalPort(port));

    AssertOk(SocketClient::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, 0, connectClient));

    AssertOk(listener.TryAccept(acceptClient));
}

class TestTcpSocket : public testing::TestWithParam<TcpSocketParam> {};

INSTANTIATE_TEST_SUITE_P(, TestTcpSocket, testing::ValuesIn(GetTcpSocketTestParameters()), [](const testing::TestParamInfo<TestTcpSocket::ParamType>& info) {
    TcpSocketParam param = info.param;
    return fmt::format("{}_{}", param.addressFamily, param.access);
});

TEST(TestTcpSocketListener, CreateWithInvalidAddressShouldFail)
{
    for (const char* address : {"", "not-an-ip", "localhost", "256.0.0.1"}) {
        // Arrange
        SocketListener listener;

        // Act
        Result result = SocketListener::Create(address, 0, listener);

        // Assert
        AssertError(result);
    }
}

TEST(TestTcpSocketListener, Ipv6AnyListenerShouldNotAcceptIpv4Connections)
{
    if (!IsIpv4SocketSupported() || !IsIpv6SocketSupported()) {
        GTEST_SKIP() << "IPv4 and IPv6 are required.";
    }

    // Arrange
    SocketListener listener;
    AssertOk(SocketListener::Create("::", 0, listener));

    uint16_t localPort{};
    AssertOk(listener.GetLocalPort(localPort));

    SocketClient client;

    // Act
    Result result = SocketClient::TryConnect("127.0.0.1", localPort, 0, 0, client);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestTcpSocket, CreateSocketShouldWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketListener listener;

    // Act
    Result result = SocketListener::Create(param.listenAddress, 0, listener);

    // Assert
    AssertOk(result);
}

TEST_P(TestTcpSocket, IsRunningAfterCreateShouldBeTrue)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketListener listener;
    AssertOk(SocketListener::Create(param.listenAddress, 0, listener));

    // Act
    bool isRunning = listener.IsRunning();

    // Assert
    ASSERT_TRUE(isRunning);
}

TEST_P(TestTcpSocket, IsNotRunningAfterStopShouldBeFalse)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketListener listener;
    AssertOk(SocketListener::Create(param.listenAddress, 0, listener));

    listener.Stop();

    // Act
    bool isRunning = listener.IsRunning();

    // Assert
    ASSERT_FALSE(isRunning);
}

TEST_P(TestTcpSocket, LocalPortIsNotZero)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketListener listener;
    AssertOk(SocketListener::Create(param.listenAddress, 0, listener));

    uint16_t localPort{};

    // Act
    Result result = listener.GetLocalPort(localPort);

    // Assert
    AssertOk(result);
    ASSERT_CHECKED_NE(localPort, 0U);
}

TEST_P(TestTcpSocket, ConnectToListeningSocketShouldWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketListener listener;
    AssertOk(SocketListener::Create(param.listenAddress, 0, listener));

    uint16_t localPort{};
    AssertOk(listener.GetLocalPort(localPort));

    SocketClient client;

    // Act
    Result result = SocketClient::TryConnect(GetLoopBackAddress(param.addressFamily), localPort, 0, 0, client);

    // Assert
    AssertOk(result);
}

TEST_P(TestTcpSocket, ConnectWithoutListeningShouldNotWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    uint16_t localPort{};

    {
        SocketListener listener;
        AssertOk(SocketListener::Create(param.listenAddress, 0, listener));

        AssertOk(listener.GetLocalPort(localPort));
    }

    SocketClient client;

    // Act
    Result result = SocketClient::TryConnect(GetLoopBackAddress(param.addressFamily), localPort, 0, 0, client);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestTcpSocket, ConnectWithLocalPortShouldUseLocalPort)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    uint16_t clientPort{};
    {
        SocketListener portFinder;
        AssertOk(SocketListener::Create(GetLoopBackAddress(param.addressFamily), 0, portFinder));
        AssertOk(portFinder.GetLocalPort(clientPort));
    }

    SocketListener listener;
    AssertOk(SocketListener::Create(param.listenAddress, 0, listener));

    uint16_t serverPort{};
    AssertOk(listener.GetLocalPort(serverPort));

    SocketClient connectClient;
    AssertOk(SocketClient::TryConnect(GetLoopBackAddress(param.addressFamily), serverPort, clientPort, 0, connectClient));

    SocketClient acceptClient;
    AssertOk(listener.TryAccept(acceptClient));

    std::string remoteAddress;

    // Act
    Result result = acceptClient.GetRemoteAddress(remoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_CHECKED_EQ(remoteAddress, fmt::format("{}:{}", GetLoopBackAddress(param.addressFamily), clientPort));
}

TEST_P(TestTcpSocket, AcceptWithoutConnectShouldNotWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketListener listener;
    AssertOk(SocketListener::Create(param.listenAddress, 0, listener));

    SocketClient client;

    // Act
    Result result = listener.TryAccept(client);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestTcpSocket, AcceptAfterStopShouldNotWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketListener listener;
    AssertOk(SocketListener::Create(param.listenAddress, 0, listener));

    listener.Stop();

    SocketClient client;

    // Act
    Result result = listener.TryAccept(client);

    // Assert
    AssertError(result);
}

TEST_P(TestTcpSocket, AcceptWithConnectShouldWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketListener listener;
    AssertOk(SocketListener::Create(param.listenAddress, 0, listener));

    uint16_t localPort{};
    AssertOk(listener.GetLocalPort(localPort));

    SocketClient connectClient;
    AssertOk(SocketClient::TryConnect(GetLoopBackAddress(param.addressFamily), localPort, 0, 0, connectClient));

    SocketClient acceptClient;

    // Act
    Result result = listener.TryAccept(acceptClient);

    // Assert
    AssertOk(result);
}

TEST_P(TestTcpSocket, WakeUpBlockingCallInConnectClientOnRemoteClient)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::promise<void> aboutToReceive;
    std::future<void> aboutToReceiveFuture = aboutToReceive.get_future();

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        aboutToReceive.set_value();
        AssertNotConnected(connectClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_CHECKED_EQ(size_t{0}, receivedSize);
    });

    ASSERT_CHECKED_EQ(aboutToReceiveFuture.wait_for(1s), std::future_status::ready);

    // Act and assert
    acceptClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_P(TestTcpSocket, WakeUpBlockingCallInConnectClientOnLocalClient)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::promise<void> aboutToReceive;
    std::future<void> aboutToReceiveFuture = aboutToReceive.get_future();

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        aboutToReceive.set_value();
        AssertNotConnected(connectClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_CHECKED_EQ(size_t{0}, receivedSize);
    });

    ASSERT_CHECKED_EQ(aboutToReceiveFuture.wait_for(1s), std::future_status::ready);

    // Act and assert
    connectClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_P(TestTcpSocket, WakeUpBlockingCallInAcceptClientOnRemoteClient)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::promise<void> aboutToReceive;
    std::future<void> aboutToReceiveFuture = aboutToReceive.get_future();

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        aboutToReceive.set_value();
        AssertNotConnected(acceptClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_CHECKED_EQ(size_t{0}, receivedSize);
    });

    ASSERT_CHECKED_EQ(aboutToReceiveFuture.wait_for(1s), std::future_status::ready);

    // Act and assert
    connectClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_P(TestTcpSocket, WakeUpBlockingCallInAcceptClientOnLocalClient)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::promise<void> aboutToReceive;
    std::future<void> aboutToReceiveFuture = aboutToReceive.get_future();

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        aboutToReceive.set_value();
        AssertNotConnected(acceptClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_CHECKED_EQ(size_t{0}, receivedSize);
    });

    ASSERT_CHECKED_EQ(aboutToReceiveFuture.wait_for(1s), std::future_status::ready);

    // Act and assert
    acceptClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_P(TestTcpSocket, IsConnectedAfterConnectAndAcceptShouldBeTrue)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act
    bool isConnected = connectClient.IsConnected();

    // Assert
    ASSERT_TRUE(isConnected);
}

TEST_P(TestTcpSocket, IsNotConnectedAfterDisconnectShouldBeFalse)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    connectClient.Disconnect();

    // Act
    bool isConnected = connectClient.IsConnected();

    // Assert
    ASSERT_FALSE(isConnected);
}

TEST_P(TestTcpSocket, RemoteAddressOnConnectClientAfterConnectAndAcceptAreValid)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::string connectClientRemoteAddress;

    // Act
    Result result = connectClient.GetRemoteAddress(connectClientRemoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_CHECKED_NE(connectClientRemoteAddress, "");
}

TEST_P(TestTcpSocket, RemoteAddressOnAcceptClientAfterConnectAndAcceptAreValid)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::string acceptClientRemoteAddress;

    // Act
    Result result = acceptClient.GetRemoteAddress(acceptClientRemoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_CHECKED_NE(acceptClientRemoteAddress, "");
}

TEST_P(TestTcpSocket, SendOnConnectClientAndReceiveOnAcceptClientShouldWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestSendAndReceive(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, PingPongBeginningWithConnectClientShouldWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestPingPong(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, SendManyElementsFromConnectClientToAcceptClientShouldWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestManyElements(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, SendBigElementFromConnectClientToAcceptClientShouldWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestBigElement(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, SendOnDisconnectedConnectClientShouldNotWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnect(connectClient);
}

TEST_P(TestTcpSocket, ReceiveOnDisconnectedConnectClientShouldNotWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnect(connectClient);
}

TEST_P(TestTcpSocket, ReceiveOnDisconnectedRemoteConnectClientShouldNotWork)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnectOnRemoteClient(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, WaitForDataTimesOutWhenNoDataArrives)
{
    // Arrange
    const TcpSocketParam& param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act
    Result result = connectClient.WaitForData(50);

    // Assert
    AssertTimeout(result);
}

}  // namespace
