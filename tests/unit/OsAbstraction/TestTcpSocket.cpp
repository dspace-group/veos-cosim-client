// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <fmt/format.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "Helper.hpp"
#include "Socket.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

struct Param {
    AddressFamily addressFamily{};
    bool enableRemoteAccess{};
};

[[nodiscard]] std::vector<Param> GetValues() {
    std::vector<Param> values;

    if (IsIpv4SocketSupported()) {
        values.push_back(Param{AddressFamily::Ipv4, true});
        values.push_back(Param{AddressFamily::Ipv4, false});
    }

    if (IsIpv6SocketSupported()) {
        values.push_back(Param{AddressFamily::Ipv6, true});
        values.push_back(Param{AddressFamily::Ipv6, false});
    }

    return values;
}

void EstablishConnection(const Param& param, SocketClient& connectClient, SocketClient& acceptClient) {
    SocketListener listener;
    AssertOk(SocketListener::Create(param.addressFamily, 0, param.enableRemoteAccess, listener));

    uint16_t port{};
    AssertOk(listener.GetLocalPort(port));

    AssertOk(SocketClient::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, 0, connectClient));

    AssertOk(listener.TryAccept(acceptClient));
}

class TestTcpSocket : public testing::TestWithParam<Param> {};

INSTANTIATE_TEST_SUITE_P(, TestTcpSocket, testing::ValuesIn(GetValues()), [](const testing::TestParamInfo<TestTcpSocket::ParamType>& info) {
    std::string access = info.param.enableRemoteAccess ? "Remote" : "Local";
    return fmt::format("{}_{}", info.param.addressFamily, access);
});

TEST_P(TestTcpSocket, CreateSocketShouldWork) {
    // Arrange
    Param param = GetParam();

    SocketListener listener;

    // Act
    Result result = SocketListener::Create(param.addressFamily, 0, param.enableRemoteAccess, listener);

    // Assert
    AssertOk(result);
}

TEST_P(TestTcpSocket, LocalPortIsNotZero) {
    // Arrange
    Param param = GetParam();

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
    Param param = GetParam();

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
    Param param = GetParam();

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
    Param param = GetParam();

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
    Param param = GetParam();

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
    Param param = GetParam();

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

TEST_P(TestTcpSocket, RemoteAddressOnConnectClientAfterConnectAndAcceptAreValid) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::string connectClientRemoteAddress;

    // Act
    Result result = connectClient.GetRemoteAddress(connectClientRemoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_STRNE(connectClientRemoteAddress.c_str(), "");
}

TEST_P(TestTcpSocket, RemoteAddressOnAcceptClientAfterConnectAndAcceptAreValid) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    std::string acceptClientRemoteAddress;

    // Act
    Result result = acceptClient.GetRemoteAddress(acceptClientRemoteAddress);

    // Assert
    AssertOk(result);
    ASSERT_STRNE(acceptClientRemoteAddress.c_str(), "");
}

TEST_P(TestTcpSocket, SendOnConnectClientAndReceiveOnAcceptClientShouldWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestSendAndReceive(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, SendOnAcceptClientAndReceiveOnConnectClientShouldWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestSendAndReceive(acceptClient, connectClient);
}

TEST_P(TestTcpSocket, PingPongBeginningWithConnectClientShouldWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestPingPong(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, PingPongBeginningWithAcceptClientShouldWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestPingPong(acceptClient, connectClient);
}

TEST_P(TestTcpSocket, SendManyElementsFromConnectClientToAcceptClientShouldWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestManyElements(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, SendManyElementsFromAcceptClientToConnectClientShouldWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestManyElements(acceptClient, connectClient);
}

TEST_P(TestTcpSocket, SendBigElementFromConnectClientToAcceptClientShouldWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestBigElement(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, SendBigElementFromAcceptClientToConnectClientShouldWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestBigElement(acceptClient, connectClient);
}

TEST_P(TestTcpSocket, SendOnDisconnectedConnectClientShouldNotWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnect(connectClient);
}

TEST_P(TestTcpSocket, SendOnDisconnectedAcceptClientShouldNotWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnect(acceptClient);
}

TEST_P(TestTcpSocket, SendOnDisconnectedRemoteConnectClientShouldNotWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnectOnRemoteClient(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, SendOnDisconnectedRemoteAcceptClientShouldNotWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestSendAfterDisconnectOnRemoteClient(acceptClient, connectClient);
}

TEST_P(TestTcpSocket, ReceiveOnDisconnectedConnectClientShouldNotWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnect(connectClient);
}

TEST_P(TestTcpSocket, ReceiveOnDisconnectedAcceptClientShouldNotWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnect(acceptClient);
}

TEST_P(TestTcpSocket, ReceiveOnDisconnectedRemoteConnectClientShouldNotWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnectOnRemoteClient(connectClient, acceptClient);
}

TEST_P(TestTcpSocket, ReceiveOnDisconnectedRemoteAcceptClientShouldNotWork) {
    // Arrange
    Param param = GetParam();

    SocketClient connectClient;
    SocketClient acceptClient;
    EstablishConnection(param, connectClient, acceptClient);

    // Act and assert
    TestReceiveAfterDisconnectOnRemoteClient(acceptClient, connectClient);
}

}  // namespace
