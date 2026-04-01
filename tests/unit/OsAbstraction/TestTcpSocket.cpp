// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "Helper.hpp"
#include "Socket.hpp"
#include "TestHelper.hpp"

using namespace std::chrono_literals;
using namespace DsVeosCoSim;

namespace {

struct TcpSocketParam {
    AddressFamily addressFamily{};
    bool enableRemoteAccess{};
};

[[nodiscard]] std::vector<TcpSocketParam> GetTcpSocketTestParameters() {
    std::vector<TcpSocketParam> values;

    if (IsIpv4SocketSupported()) {
        values.push_back(TcpSocketParam{AddressFamily::Ipv4, true});
        values.push_back(TcpSocketParam{AddressFamily::Ipv4, false});
    }

    if (IsIpv6SocketSupported()) {
        values.push_back(TcpSocketParam{AddressFamily::Ipv6, true});
        values.push_back(TcpSocketParam{AddressFamily::Ipv6, false});
    }

    return values;
}

void EstablishConnection(const TcpSocketParam& param, SocketClient& connectClient, SocketClient& acceptClient) {
    SocketListener listener;
    AssertOk(SocketListener::Create(param.addressFamily, 0, param.enableRemoteAccess, listener));

    uint16_t port{};
    AssertOk(listener.GetLocalPort(port));

    AssertOk(SocketClient::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, 0, connectClient));

    AssertOk(listener.TryAccept(acceptClient));
}

class TestTcpSocket : public testing::TestWithParam<TcpSocketParam> {};

INSTANTIATE_TEST_SUITE_P(, TestTcpSocket, testing::ValuesIn(GetTcpSocketTestParameters()), [](const testing::TestParamInfo<TestTcpSocket::ParamType>& info) {
    TcpSocketParam param = info.param;
    std::string access = param.enableRemoteAccess ? "Remote" : "Local";
    return fmt::format("{}_{}", param.addressFamily, access);
});

TEST_P(TestTcpSocket, CreateSocketShouldWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketListener listener;

    // Act
    Result result = SocketListener::Create(param.addressFamily, 0, param.enableRemoteAccess, listener);

    // Assert
    AssertOk(result);
}

TEST_P(TestTcpSocket, LocalPortIsNotZero) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketListener listener;
    AssertOk(SocketListener::Create(param.addressFamily, 0, param.enableRemoteAccess, listener));

    uint16_t localPort{};

    // Act
    Result result = listener.GetLocalPort(localPort);

    // Assert
    AssertOk(result);
    ASSERT_NE(localPort, 0U);
}

TEST_P(TestTcpSocket, ConnectToListeningSocketShouldWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketListener listener;
    AssertOk(SocketListener::Create(param.addressFamily, 0, param.enableRemoteAccess, listener));

    uint16_t localPort{};
    AssertOk(listener.GetLocalPort(localPort));

    SocketClient client;

    // Act
    Result result = SocketClient::TryConnect(GetLoopBackAddress(param.addressFamily), localPort, 0, 0, client);

    // Assert
    AssertOk(result);
}

TEST_P(TestTcpSocket, ConnectWithoutListeningShouldNotWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    uint16_t localPort{};

    {
        SocketListener listener;
        AssertOk(SocketListener::Create(param.addressFamily, 0, param.enableRemoteAccess, listener));

        AssertOk(listener.GetLocalPort(localPort));
    }

    SocketClient client;

    // Act
    Result result = SocketClient::TryConnect(GetLoopBackAddress(param.addressFamily), localPort, 0, 0, client);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestTcpSocket, AcceptWithoutConnectShouldNotWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketListener listener;
    AssertOk(SocketListener::Create(param.addressFamily, 0, param.enableRemoteAccess, listener));

    SocketClient client;

    // Act
    Result result = listener.TryAccept(client);

    // Assert
    AssertNotConnected(result);
}

TEST_P(TestTcpSocket, AcceptAfterStopShouldNotWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketListener listener;
    AssertOk(SocketListener::Create(param.addressFamily, 0, param.enableRemoteAccess, listener));

    listener.Stop();

    SocketClient client;

    // Act
    Result result = listener.TryAccept(client);

    // Assert
    AssertError(result);
}

TEST_P(TestTcpSocket, AcceptWithConnectShouldWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketListener listener;
    AssertOk(SocketListener::Create(param.addressFamily, 0, param.enableRemoteAccess, listener));

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

TEST_P(TestTcpSocket, WakeUpBlockingCallInConnectClientOnRemoteClient) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        AssertNotConnected(connectClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_EQ(0, receivedSize);
    });

    std::this_thread::sleep_for(100ms);

    // Act and assert
    acceptClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_P(TestTcpSocket, WakeUpBlockingCallInAcceptClientOnRemoteClient) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        AssertNotConnected(acceptClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_EQ(0, receivedSize);
    });

    std::this_thread::sleep_for(100ms);

    // Act and assert
    connectClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_P(TestTcpSocket, WakeUpBlockingCallInConnectClientOnLocalClient) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        AssertNotConnected(connectClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_EQ(0, receivedSize);
    });

    std::this_thread::sleep_for(100ms);

    // Act and assert
    connectClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_P(TestTcpSocket, WakeUpBlockingCallInAcceptClientOnLocalClient) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::thread t([&] {
        std::array<uint8_t, 10> buffer{};
        size_t receivedSize{};
        AssertNotConnected(acceptClient.Receive(buffer.data(), buffer.size(), receivedSize));
        ASSERT_EQ(0, receivedSize);
    });

    std::this_thread::sleep_for(100ms);

    // Act and assert
    acceptClient.Disconnect();

    // Cleanup
    t.join();
}

TEST_P(TestTcpSocket, RemoteAddressOnConnectClientAfterConnectAndAcceptAreValid) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::string connectClientRemoteAddress;

    // Act
    Result result = connectClient.GetRemoteAddress(connectClientRemoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_NE(connectClientRemoteAddress, "");
}

TEST_P(TestTcpSocket, RemoteAddressOnAcceptClientAfterConnectAndAcceptAreValid) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::string acceptClientRemoteAddress;

    // Act
    Result result = acceptClient.GetRemoteAddress(acceptClientRemoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_NE(acceptClientRemoteAddress, "");
}

TEST_P(TestTcpSocket, SendOnConnectClientAndReceiveOnAcceptClientShouldWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestSendAndReceive(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, SendOnAcceptClientAndReceiveOnConnectClientShouldWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestSendAndReceive(acceptClient, connectClient);
}

TEST_P(TestTcpSocket, PingPongBeginningWithConnectClientShouldWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestPingPong(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, PingPongBeginningWithAcceptClientShouldWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestPingPong(acceptClient, connectClient);
}

TEST_P(TestTcpSocket, SendManyElementsFromConnectClientToAcceptClientShouldWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestManyElements(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, SendManyElementsFromAcceptClientToConnectClientShouldWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestManyElements(acceptClient, connectClient);
}

TEST_P(TestTcpSocket, SendBigElementFromConnectClientToAcceptClientShouldWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestBigElement(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, SendBigElementFromAcceptClientToConnectClientShouldWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestBigElement(acceptClient, connectClient);
}

TEST_P(TestTcpSocket, SendOnDisconnectedConnectClientShouldNotWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnect(connectClient);
}

TEST_P(TestTcpSocket, SendOnDisconnectedAcceptClientShouldNotWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnect(acceptClient);
}

TEST_P(TestTcpSocket, ReceiveOnDisconnectedConnectClientShouldNotWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnect(connectClient);
}

TEST_P(TestTcpSocket, ReceiveOnDisconnectedAcceptClientShouldNotWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnect(acceptClient);
}

TEST_P(TestTcpSocket, ReceiveOnDisconnectedRemoteConnectClientShouldNotWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnectOnRemoteClient(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, ReceiveOnDisconnectedRemoteAcceptClientShouldNotWork) {
    // Arrange
    TcpSocketParam param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnectOnRemoteClient(acceptClient, connectClient);
}

}  // namespace
