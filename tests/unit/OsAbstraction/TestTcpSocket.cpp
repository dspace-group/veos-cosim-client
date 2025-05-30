// Copyright dSPACE GmbH. All rights reserved.

#include <fmt/format.h>

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

#include "Generator.h"
#include "Helper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

struct Param {
    AddressFamily addressFamily{};
    bool enableRemoteAccess{};
};

[[nodiscard]] std::vector<Param> GetValues() {
    std::vector<Param> values;

    if (Socket::IsIpv4Supported()) {
        values.push_back(Param{AddressFamily::Ipv4, true});
        values.push_back(Param{AddressFamily::Ipv4, false});
    }

    if (Socket::IsIpv6Supported()) {
        values.push_back(Param{AddressFamily::Ipv6, true});
        values.push_back(Param{AddressFamily::Ipv6, false});
    }

    return values;
}

class TestTcpSocket : public testing::TestWithParam<Param> {};

INSTANTIATE_TEST_SUITE_P(,
                         TestTcpSocket,
                         testing::ValuesIn(GetValues()),
                         [](const testing::TestParamInfo<TestTcpSocket::ParamType>& info) {
                             std::string access = info.param.enableRemoteAccess ? "Remote" : "Local";
                             return fmt::format("{}_{}", ToString(info.param.addressFamily), access);
                         });

TEST_P(TestTcpSocket, Create) {
    // Arrange
    const Param param = GetParam();

    // Act and assert
    ASSERT_NO_THROW(Socket(param.addressFamily));
}

TEST_P(TestTcpSocket, Bind) {
    // Arrange
    const Param param = GetParam();

    const Socket serverSocket(param.addressFamily);

    // Act and assert
    ASSERT_NO_THROW(serverSocket.Bind(0, param.enableRemoteAccess));
}

TEST_P(TestTcpSocket, LocalPortIsNotZeroAfterBind) {
    // Arrange
    const Param param = GetParam();

    const Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);

    // Act
    const uint16_t localPort = serverSocket.GetLocalPort();

    // Assert
    ASSERT_NE(localPort, 0);
}

TEST_P(TestTcpSocket, Listen) {
    // Arrange
    const Param param = GetParam();

    const Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);

    // Act and assert
    ASSERT_NO_THROW(serverSocket.Listen());
}

#ifdef _WIN32
TEST_P(TestTcpSocket, ConnectWithoutListening) {
    // Arrange
    const Param param = GetParam();

    const Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);
    const uint16_t port = serverSocket.GetLocalPort();

    // Act
    const std::optional<Socket> connectedSocket =
        Socket::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, 0);

    // Assert
    ASSERT_FALSE(connectedSocket);
}
#endif

TEST_P(TestTcpSocket, Connect) {
    // Arrange
    const Param param = GetParam();

    const Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);
    const uint16_t port = serverSocket.GetLocalPort();
    serverSocket.Listen();

    // Act
    const std::optional<Socket> connectedSocket =
        Socket::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, DefaultTimeout);

    // Assert
    ASSERT_TRUE(connectedSocket);
}

TEST_P(TestTcpSocket, AcceptWithoutConnect) {
    // Arrange
    const Param param = GetParam();

    const Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);
    serverSocket.Listen();

    // Act
    const std::optional<Socket> acceptedSocket = serverSocket.TryAccept(0);

    // Assert
    ASSERT_FALSE(acceptedSocket);
}

TEST_P(TestTcpSocket, Accept) {
    // Arrange
    const Param param = GetParam();

    const Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);
    const uint16_t port = serverSocket.GetLocalPort();
    serverSocket.Listen();

    const std::optional<Socket> connectedSocket =
        Socket::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedSocket);

    // Act
    const std::optional<Socket> acceptedSocket = serverSocket.TryAccept(DefaultTimeout);

    // Assert
    ASSERT_TRUE(acceptedSocket);
}

TEST_P(TestTcpSocket, PortsAfterConnectAndAccept) {
    // Arrange
    const Param param = GetParam();

    const Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);
    const uint16_t port = serverSocket.GetLocalPort();
    serverSocket.Listen();

    const std::optional<Socket> connectedSocket =
        Socket::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedSocket);

    const std::optional<Socket> acceptedSocket = serverSocket.TryAccept(DefaultTimeout);
    EXPECT_TRUE(acceptedSocket);

    // Act
    const uint16_t connectedSocketLocalPort = connectedSocket->GetLocalPort();

    // Assert
    ASSERT_NE(connectedSocketLocalPort, port);
#ifdef _WIN32
    ASSERT_EQ(connectedSocketLocalPort, acceptedSocket->GetRemoteAddress().port);
    ASSERT_EQ(acceptedSocket->GetLocalPort(), connectedSocket->GetRemoteAddress().port);
#endif
}

TEST_P(TestTcpSocket, SendAndReceive) {
    // Arrange
    const Param param = GetParam();

    const Socket serverSocket(param.addressFamily);
    serverSocket.Bind(0, param.enableRemoteAccess);
    const uint16_t port = serverSocket.GetLocalPort();
    serverSocket.Listen();

    const std::optional<Socket> connectedSocket =
        Socket::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, DefaultTimeout);
    EXPECT_TRUE(connectedSocket);

    const std::optional<Socket> acceptedSocket = serverSocket.TryAccept(DefaultTimeout);
    EXPECT_TRUE(acceptedSocket);

    const uint32_t sendValue = GenerateU32();
    uint32_t receiveValue = 0;

    // Act
    ASSERT_TRUE(SendComplete(*connectedSocket, &sendValue, sizeof(sendValue)));
    ASSERT_TRUE(ReceiveComplete(*acceptedSocket, &receiveValue, sizeof(receiveValue)));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}

}  // namespace
