// Copyright dSPACE GmbH. All rights reserved.

#include <gtest/gtest.h>

#include "Generator.h"
#include "Helper.h"
#include "Socket.h"

using namespace DsVeosCoSim;

namespace {

class TestUdsSocket : public testing::Test {};

TEST_F(TestUdsSocket, Create) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;

    // Act and assert
    ASSERT_NO_THROW((void)Socket(addressFamily));
}

TEST_F(TestUdsSocket, Bind) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;
    const std::string path = GenerateString("UdsPath");

    Socket serverSocket(addressFamily);

    // Act and assert
    ASSERT_NO_THROW(serverSocket.Bind(path));
}

TEST_F(TestUdsSocket, Listen) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;
    const std::string path = GenerateString("UdsPath");

    Socket serverSocket(addressFamily);
    serverSocket.Bind(path);

    // Act and assert
    ASSERT_NO_THROW(serverSocket.Listen());
}

TEST_F(TestUdsSocket, ConnectWithoutListening) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;
    const std::string path = GenerateString("UdsPath");

    Socket serverSocket(addressFamily);
    serverSocket.Bind(path);

    const Socket clientSocket(addressFamily);

    // Act
    const bool result = clientSocket.TryConnect(path);

    // Assert
    ASSERT_FALSE(result);
}

TEST_F(TestUdsSocket, Connect) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;
    const std::string path = GenerateString("UdsPath");

    Socket serverSocket(addressFamily);
    serverSocket.Bind(path);
    serverSocket.Listen();

    const Socket clientSocket(addressFamily);

    // Act
    const bool result = clientSocket.TryConnect(path);

    // Assert
    ASSERT_TRUE(result);
}

TEST_F(TestUdsSocket, Accept) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;
    const std::string path = GenerateString("UdsPath");

    Socket serverSocket(addressFamily);
    serverSocket.Bind(path);
    serverSocket.Listen();

    const Socket clientSocket(addressFamily);
    EXPECT_TRUE(clientSocket.TryConnect(path));

    // Act
    const std::optional<Socket> acceptedSocket = serverSocket.TryAccept();

    // Assert
    ASSERT_TRUE(acceptedSocket);
}

TEST_F(TestUdsSocket, SendAndReceive) {
    // Arrange
    constexpr auto addressFamily = AddressFamily::Uds;
    const std::string path = GenerateString("UdsPath");

    Socket serverSocket(addressFamily);
    serverSocket.Bind(path);
    serverSocket.Listen();

    const Socket clientSocket(addressFamily);
    EXPECT_TRUE(clientSocket.TryConnect(path));

    const Socket acceptedSocket = Accept(serverSocket);

    const uint32_t sendValue = GenerateU32();
    uint32_t receiveValue = 0;

    // Act
    ASSERT_TRUE(SendComplete(clientSocket, &sendValue, sizeof(sendValue)));
    ASSERT_TRUE(ReceiveComplete(acceptedSocket, &receiveValue, sizeof(receiveValue)));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}

}  // namespace
