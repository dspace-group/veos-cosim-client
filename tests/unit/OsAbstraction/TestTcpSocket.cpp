// Copyright dSPACE GmbH. All rights reserved.

#include <fmt/format.h>

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

#include "Generator.h"
#include "Helper.h"
#include "Socket.h"
#include "TestHelper.h"

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
    Param param = GetParam();

    Socket socket;

    // Act and assert
    AssertOk(Socket::Create(param.addressFamily, socket));
}

TEST_P(TestTcpSocket, Bind) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket;
    ExpectOk(Socket::Create(param.addressFamily, serverSocket));

    // Act and assert
    AssertOk(serverSocket.Bind(0, param.enableRemoteAccess));
}

TEST_P(TestTcpSocket, LocalPortIsNotZeroAfterBind) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket;
    ExpectOk(Socket::Create(param.addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(0, param.enableRemoteAccess));

    uint16_t localPort{};

    // Act
    AssertOk(serverSocket.GetLocalPort(localPort));

    // Assert
    AssertNotEq(uint16_t(0), localPort);
}

TEST_P(TestTcpSocket, Listen) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket;
    ExpectOk(Socket::Create(param.addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(0, param.enableRemoteAccess));

    // Act and assert
    AssertOk(serverSocket.Listen());
}

#ifdef _WIN32
TEST_P(TestTcpSocket, ConnectWithoutListening) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket;
    ExpectOk(Socket::Create(param.addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(0, param.enableRemoteAccess));
    uint16_t port{};
    ExpectOk(serverSocket.GetLocalPort(port));

    std::optional<Socket> connectedSocket;

    // Act
    AssertOk(Socket::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, 0, connectedSocket));

    // Assert
    AssertFalse(connectedSocket);
}
#endif

TEST_P(TestTcpSocket, Connect) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket;
    ExpectOk(Socket::Create(param.addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(0, param.enableRemoteAccess));
    uint16_t port{};
    ExpectOk(serverSocket.GetLocalPort(port));
    ExpectOk(serverSocket.Listen());

    std::optional<Socket> connectedSocket;

    // Act
    AssertOk(Socket::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, 0, connectedSocket));

    // Assert
    AssertTrue(connectedSocket);
}

TEST_P(TestTcpSocket, AcceptWithoutConnect) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket;
    ExpectOk(Socket::Create(param.addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(0, param.enableRemoteAccess));
    ExpectOk(serverSocket.Listen());

    std::optional<Socket> acceptedSocket;

    // Act
    AssertOk(serverSocket.TryAccept(acceptedSocket));

    // Assert
    AssertFalse(acceptedSocket);
}

TEST_P(TestTcpSocket, Accept) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket;
    ExpectOk(Socket::Create(param.addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(0, param.enableRemoteAccess));
    uint16_t port{};
    ExpectOk(serverSocket.GetLocalPort(port));
    ExpectOk(serverSocket.Listen());

    std::optional<Socket> connectedSocket;
    ExpectOk(Socket::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, DefaultTimeout, connectedSocket));
    ExpectTrue(connectedSocket);

    std::optional<Socket> acceptedSocket;

    // Act
    AssertOk(serverSocket.TryAccept(acceptedSocket));

    // Assert
    AssertTrue(acceptedSocket);
}

TEST_P(TestTcpSocket, PortsAfterConnectAndAccept) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket;
    ExpectOk(Socket::Create(param.addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(0, param.enableRemoteAccess));
    uint16_t port{};
    ExpectOk(serverSocket.GetLocalPort(port));
    ExpectOk(serverSocket.Listen());

    std::optional<Socket> connectedSocket;
    ExpectOk(Socket::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, DefaultTimeout, connectedSocket));
    ExpectTrue(connectedSocket);

    std::optional<Socket> acceptedSocket;
    ExpectOk(serverSocket.TryAccept(acceptedSocket));
    ExpectTrue(acceptedSocket);

    uint16_t connectedSocketLocalPort{};
#ifdef _WIN32
    SocketAddress remoteAddressForAcceptedSocket;
    SocketAddress remoteAddressForConnectedSocket;
    uint16_t localPort{};
#endif

    // Act
    AssertOk(connectedSocket->GetLocalPort(connectedSocketLocalPort));
#ifdef _WIN32
    AssertOk(acceptedSocket->GetRemoteAddress(remoteAddressForAcceptedSocket));
    AssertOk(acceptedSocket->GetLocalPort(localPort));
    AssertOk(connectedSocket->GetRemoteAddress(remoteAddressForConnectedSocket));
#endif

    // Assert
    AssertNotEq(connectedSocketLocalPort, port);
#ifdef _WIN32
    AssertEq(connectedSocketLocalPort, remoteAddressForAcceptedSocket.port);
    AssertEq(localPort, remoteAddressForConnectedSocket.port);
#endif
}

TEST_P(TestTcpSocket, SendAndReceive) {
    // Arrange
    Param param = GetParam();

    Socket serverSocket;
    ExpectOk(Socket::Create(param.addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(0, param.enableRemoteAccess));
    uint16_t port{};
    ExpectOk(serverSocket.GetLocalPort(port));
    ExpectOk(serverSocket.Listen());

    std::optional<Socket> connectedSocket;
    ExpectOk(Socket::TryConnect(GetLoopBackAddress(param.addressFamily), port, 0, DefaultTimeout, connectedSocket));
    ExpectTrue(connectedSocket);

    std::optional<Socket> acceptedSocket;
    ExpectOk(serverSocket.TryAccept(acceptedSocket));
    ExpectTrue(acceptedSocket);

    uint32_t sendValue = GenerateU32();
    uint32_t receiveValue = 0;

    // Act
    AssertOk(SendComplete(*connectedSocket, &sendValue, sizeof(sendValue)));
    AssertOk(ReceiveComplete(*acceptedSocket, &receiveValue, sizeof(receiveValue)));

    // Assert
    AssertEq(sendValue, receiveValue);
}

}  // namespace
