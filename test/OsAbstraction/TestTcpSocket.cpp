// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include "Generator.h"
#include "Helper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

struct Param {
    AddressFamily addressFamily{};
    bool enableRemoteAccess{};
};

}  // namespace

class TestTcpSocket : public testing::TestWithParam<Param> {};

INSTANTIATE_TEST_SUITE_P(,
                         TestTcpSocket,
                         testing::Values(Param{AddressFamily::Ipv4, true},
                                         Param{AddressFamily::Ipv4, false},
                                         Param{AddressFamily::Ipv6, true},
                                         Param{AddressFamily::Ipv6, false}),
                         [](const testing::TestParamInfo<TestTcpSocket::ParamType>& info) {
                             std::string access = info.param.enableRemoteAccess ? "Remote" : "Local";
                             return fmt::format("{}_{}", info.param.addressFamily, access);
                         });

TEST_P(TestTcpSocket, Create) {
    // Arrange
    Param param = GetParam();

    // Act and assert
    ASSERT_NO_THROW(Socket(param.addressFamily));
}

TEST_P(TestTcpSocket, Bind) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket(param.addressFamily);

    // Act and assert
    ASSERT_NO_THROW(serverSocket.Bind(0, param.enableRemoteAccess));
}

TEST_P(TestTcpSocket, LocalPortIsNotZeroAfterBind) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);

    // Act
    uint16_t localPort = serverSocket.GetLocalPort();

    // Assert
    ASSERT_NE(localPort, 0);
}

TEST_P(TestTcpSocket, Listen) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);

    // Act and assert
    ASSERT_NO_THROW(serverSocket.Listen());
}

TEST_P(TestTcpSocket, ConnectWithoutListening) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);
    uint16_t port = serverSocket.GetLocalPort();

    // Act
    std::optional<Socket> connectedSocket = Socket::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, 0);

    // Assert
    ASSERT_FALSE(connectedSocket);
}

TEST_P(TestTcpSocket, Connect) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);
    uint16_t port = serverSocket.GetLocalPort();
    serverSocket.Listen();

    // Act
    std::optional<Socket> connectedSocket =
        Socket::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, DefaultTimeout);

    // Assert
    ASSERT_TRUE(connectedSocket);
}

TEST_P(TestTcpSocket, AcceptWithoutConnect) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);
    serverSocket.Listen();

    // Act
    std::optional<Socket> acceptedSocket = serverSocket.TryAccept();

    // Assert
    ASSERT_FALSE(acceptedSocket);
}

TEST_P(TestTcpSocket, Accept) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);
    uint16_t port = serverSocket.GetLocalPort();
    serverSocket.Listen();

    (void)ConnectSocket(GetLoopBackAddress(param.addressFamily), port);

    // Act
    std::optional<Socket> acceptedSocket = serverSocket.TryAccept(DefaultTimeout);

    // Assert
    ASSERT_TRUE(acceptedSocket);
}

TEST_P(TestTcpSocket, PortsAfterConnectAndAccept) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);
    uint16_t port = serverSocket.GetLocalPort();
    serverSocket.Listen();

    Socket connectedSocket = ConnectSocket(GetLoopBackAddress(param.addressFamily), port);

    Socket acceptedSocket = Accept(serverSocket);

    // Act
    uint16_t connectedSocketLocalPort = connectedSocket.GetLocalPort();

    // Assert
    ASSERT_NE(connectedSocketLocalPort, port);
#ifdef _WIN32
    ASSERT_EQ(connectedSocketLocalPort, acceptedSocket.GetRemoteAddress().port);
    ASSERT_EQ(acceptedSocket.GetLocalPort(), connectedSocket.GetRemoteAddress().port);
#endif
}

TEST_P(TestTcpSocket, SendAndReceive) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);
    uint16_t port = serverSocket.GetLocalPort();
    serverSocket.Listen();

    Socket connectedSocket = ConnectSocket(GetLoopBackAddress(param.addressFamily), port);

    Socket acceptedSocket = Accept(serverSocket);

    const uint32_t sendValue = GenerateU32();
    uint32_t receiveValue = 0;

    // Act
    ASSERT_TRUE(SendComplete(connectedSocket, &sendValue, sizeof(sendValue)));
    ASSERT_TRUE(ReceiveComplete(acceptedSocket, &receiveValue, sizeof(receiveValue)));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}
