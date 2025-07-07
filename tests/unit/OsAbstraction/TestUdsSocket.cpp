// Copyright dSPACE GmbH. All rights reserved.

#include <optional>

#include <gtest/gtest.h>

#include "Helper.h"
#include "Socket.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

namespace {

class TestUdsSocket : public testing::Test {};

TEST_F(TestUdsSocket, Create) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;

    Socket socket;

    // Act
    AssertOk(Socket::Create(addressFamily, socket));

    // Assert
    AssertTrue(socket.IsValid());
}

TEST_F(TestUdsSocket, Bind) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;
    std::string path = GenerateString("UdsPath");

    Socket serverSocket;
    ExpectOk(Socket::Create(addressFamily, serverSocket));

    // Act and assert
    AssertOk(serverSocket.Bind(path));
}

TEST_F(TestUdsSocket, Listen) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;
    std::string path = GenerateString("UdsPath");

    Socket serverSocket;
    ExpectOk(Socket::Create(addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(path));

    // Act and assert
    AssertOk(serverSocket.Listen());
}

TEST_F(TestUdsSocket, ConnectWithoutListening) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;
    std::string path = GenerateString("UdsPath");

    Socket serverSocket;
    ExpectOk(Socket::Create(addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(path));

    std::optional<Socket> connectedSocket;

    // Act and assert
    AssertOk(Socket::TryConnect(path, connectedSocket));

    // Assert
    AssertFalse(connectedSocket);
}

TEST_F(TestUdsSocket, Connect) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;
    std::string path = GenerateString("UdsPath");

    Socket serverSocket;
    ExpectOk(Socket::Create(addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(path));
    ExpectOk(serverSocket.Listen());

    Socket clientSocket;
    ExpectOk(Socket::Create(addressFamily, clientSocket));

    std::optional<Socket> connectedSocket;

    // Act
    AssertOk(Socket::TryConnect(path, connectedSocket));

    // Assert
    AssertTrue(connectedSocket);
}

TEST_F(TestUdsSocket, Accept) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;
    std::string path = GenerateString("UdsPath");

    Socket serverSocket;
    ExpectOk(Socket::Create(addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(path));
    ExpectOk(serverSocket.Listen());

    std::optional<Socket> clientSocket;
    ExpectOk(Socket::TryConnect(path, clientSocket));
    ExpectTrue(clientSocket);

    std::optional<Socket> acceptedSocket;

    // Act
    AssertOk(serverSocket.TryAccept(acceptedSocket));

    // Assert
    AssertTrue(acceptedSocket);
}

TEST_F(TestUdsSocket, SendAndReceive) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;
    std::string path = GenerateString("UdsPath");

    Socket serverSocket;
    ExpectOk(Socket::Create(addressFamily, serverSocket));
    ExpectOk(serverSocket.Bind(path));
    ExpectOk(serverSocket.Listen());

    std::optional<Socket> clientSocket;
    ExpectOk(Socket::TryConnect(path, clientSocket));
    ExpectTrue(clientSocket);

    std::optional<Socket> acceptedSocket;
    ExpectOk(serverSocket.TryAccept(acceptedSocket));
    ExpectTrue(acceptedSocket);

    uint32_t sendValue = GenerateU32();
    uint32_t receiveValue = 0;

    // Act
    AssertOk(SendComplete(*clientSocket, &sendValue, sizeof(sendValue)));
    AssertOk(ReceiveComplete(*acceptedSocket, &receiveValue, sizeof(receiveValue)));

    // Assert
    AssertEq(sendValue, receiveValue);
}

}  // namespace
