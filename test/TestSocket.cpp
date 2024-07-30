// Copyright dSPACE GmbH. All rights reserved.

#include "Generator.h"
#include "Logger.h"
#include "Socket.h"
#include "TestHelper.h"

using namespace std::chrono_literals;
using namespace DsVeosCoSim;

namespace {

void SendExactly(const Socket& socket, const void* buffer, size_t length) {
    const auto* bufferPointer = (const uint8_t*)buffer;
    while (length > 0) {
        int sentSize = 0;
        ASSERT_OK(socket.Send(bufferPointer, (int)length, sentSize));

        length -= sentSize;
        bufferPointer += sentSize;
    }
}

void ReceiveExactly(const Socket& socket, void* buffer, size_t length) {
    auto* bufferPointer = (uint8_t*)buffer;
    while (length > 0) {
        int receivedSize = 0;
        ASSERT_OK(socket.Receive(bufferPointer, (int)length, receivedSize));

        length -= receivedSize;
        bufferPointer += receivedSize;
    }
}

struct Param {
    AddressFamily addressFamily{};
    bool enableRemoteAccess{};
};

std::string GetLoopBackAddress(AddressFamily addressFamily) {
    if (addressFamily == DsVeosCoSim::AddressFamily::Ipv4) {
        return "127.0.0.1";
    }

    return "::1";
}

}  // namespace

class TcpSocket : public testing::TestWithParam<Param> {
protected:
    void SetUp() override {
        SetLogCallback(OnLogCallback);

        ClearLastMessage();
    }
};

INSTANTIATE_TEST_SUITE_P(
    Test,
    TcpSocket,
    testing::Values(Param{AddressFamily::Ipv4, true}, Param{AddressFamily::Ipv4, false}, Param{AddressFamily::Ipv6, true}, Param{AddressFamily::Ipv6, false}),
    [](const testing::TestParamInfo<TcpSocket::ParamType>& info) {
        return ToString(info.param.addressFamily) + "_" + ToString(info.param.enableRemoteAccess);
    });

TEST_P(TcpSocket, Create) {
    // Arrange
    Param param = GetParam();
    Socket server;

    // Act
    Result result = server.Create(param.addressFamily);

    // Assert
    ASSERT_OK(result);
}

TEST_P(TcpSocket, Bind) {
    // Arrange
    Param param = GetParam();
    Socket server;
    ASSERT_OK(server.Create(param.addressFamily));
    uint16_t localPort{};

    // Act
    Result bindResult = server.Bind(0, param.enableRemoteAccess);
    Result getLocalPortResult = server.GetLocalPort(localPort);

    // Assert
    ASSERT_OK(bindResult);
    ASSERT_OK(getLocalPortResult);
    ASSERT_NE(localPort, 0);
}

TEST_P(TcpSocket, Listen) {
    // Arrange
    Param param = GetParam();
    Socket server;
    ASSERT_OK(server.Create(param.addressFamily));
    ASSERT_OK(server.Bind(0, param.enableRemoteAccess));

    // Act
    Result result = server.Listen();

    // Assert
    ASSERT_OK(result);
}

TEST_P(TcpSocket, Connect) {  // NOLINT(readability-function-cognitive-complexity)
    // Arrange
    Param param = GetParam();
    Socket server;
    Socket client;
    uint16_t port{};
    ASSERT_OK(server.Create(param.addressFamily));
    ASSERT_OK(server.Bind(port, param.enableRemoteAccess));
    ASSERT_OK(server.Listen());
    ASSERT_OK(server.GetLocalPort(port));
    ASSERT_OK(client.Create(param.addressFamily));

    // Act
    Result result = client.Connect(GetLoopBackAddress(param.addressFamily), port, 0);

    // Assert
    ASSERT_OK(result);
}

TEST_P(TcpSocket, Accept) {  // NOLINT(readability-function-cognitive-complexity)
    // Arrange
    Param param = GetParam();
    Socket server;
    Socket client;
    uint16_t localClientPort{};
    uint16_t remoteClientPort{};
    uint16_t localServerPort{};
    uint16_t localAcceptedClientPort{};
    uint16_t remoteAcceptedClientPort{};
    ASSERT_OK(server.Create(param.addressFamily));
    ASSERT_OK(server.Bind(localServerPort, param.enableRemoteAccess));
    ASSERT_OK(server.GetLocalPort(localServerPort));
    ASSERT_OK(server.Listen());
    ASSERT_OK(client.Create(param.addressFamily));
    ASSERT_OK(client.Connect(GetLoopBackAddress(param.addressFamily), localServerPort, 0));
    Socket acceptedClient;
    std::string remoteClientAddress;
    std::string remoteAcceptedClientAddress;

    // Act
    Result acceptResult = server.Accept(acceptedClient);
    Result getLocalClientPortResult = client.GetLocalPort(localClientPort);
    Result getRemoteClientPortResult = client.GetRemoteAddress(remoteClientAddress, remoteClientPort);
    Result getLocalAcceptedClientPortResult = acceptedClient.GetLocalPort(localAcceptedClientPort);
    Result getRemoteAcceptedClientPortResult = acceptedClient.GetRemoteAddress(remoteAcceptedClientAddress, remoteAcceptedClientPort);
    Result getLocalServerPortResult = server.GetLocalPort(localServerPort);

    // Assert
    ASSERT_OK(acceptResult);
    ASSERT_OK(getLocalClientPortResult);
    ASSERT_OK(getRemoteClientPortResult);
    ASSERT_OK(getLocalAcceptedClientPortResult);
    ASSERT_OK(getRemoteAcceptedClientPortResult);
    ASSERT_OK(getLocalServerPortResult);
    ASSERT_NE(localClientPort, 0);
    ASSERT_NE(localServerPort, 0);
    ASSERT_NE(localClientPort, localServerPort);
    ASSERT_EQ(localClientPort, remoteAcceptedClientPort);
    ASSERT_EQ(localServerPort, localAcceptedClientPort);
    ASSERT_EQ(localServerPort, remoteClientPort);
}

TEST_P(TcpSocket, SendAndReceive) {  // NOLINT(readability-function-cognitive-complexity)
    // Arrange
    Param param = GetParam();
    Socket server;
    Socket client;
    uint16_t port{};
    ASSERT_OK(server.Create(param.addressFamily));
    ASSERT_OK(server.Bind(0, false));
    ASSERT_OK(server.Listen());
    ASSERT_OK(server.GetLocalPort(port));
    ASSERT_OK(client.Create(param.addressFamily));
    ASSERT_OK(client.Connect(GetLoopBackAddress(param.addressFamily), port, 0));
    Socket acceptedClient;
    ASSERT_OK(server.Accept(acceptedClient));

    const uint32_t sendValue = GenerateU32();
    uint32_t receiveValue = 0;

    // Act
    SendExactly(client, &sendValue, sizeof(sendValue));
    ReceiveExactly(acceptedClient, &receiveValue, sizeof(receiveValue));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}

class UdsSocket : public testing::Test {
protected:
    void SetUp() override {
        SetLogCallback(OnLogCallback);

        ClearLastMessage();
    }
};

TEST_F(UdsSocket, Create) {
    // Arrange
    Socket server;

    // Act
    Result result = server.Create(AddressFamily::Uds);

    // Assert
    ASSERT_OK(result);
}

TEST_F(UdsSocket, Bind) {
    // Arrange
    Socket server;
    std::string path = GenerateString("UdsPath");
    ASSERT_OK(server.Create(AddressFamily::Uds));

    // Act
    Result result = server.Bind(path);

    // Assert
    ASSERT_OK(result);
}

TEST_F(UdsSocket, Listen) {
    // Arrange
    Socket server;
    std::string path = GenerateString("UdsPath");
    ASSERT_OK(server.Create(AddressFamily::Uds));
    ASSERT_OK(server.Bind(path));

    // Act
    Result result = server.Listen();

    // Assert
    ASSERT_OK(result);
}

TEST_F(UdsSocket, Connect) {
    // Arrange
    Socket server;
    Socket client;
    std::string path = GenerateString("UdsPath");
    ASSERT_OK(server.Create(AddressFamily::Uds));
    ASSERT_OK(server.Bind(path));
    ASSERT_OK(server.Listen());
    ASSERT_OK(client.Create(AddressFamily::Uds));

    // Act
    Result result = client.Connect(path);

    // Assert
    ASSERT_OK(result);
}

TEST_F(UdsSocket, Accept) {
    // Arrange
    Socket server;
    Socket client;
    std::string path = GenerateString("UdsPath");
    ASSERT_OK(server.Create(AddressFamily::Uds));
    ASSERT_OK(server.Bind(path));
    ASSERT_OK(server.Listen());
    ASSERT_OK(client.Create(AddressFamily::Uds));
    ASSERT_OK(client.Connect(path));
    Socket acceptedClient;

    // Act
    Result result = server.Accept(acceptedClient);

    // Assert
    ASSERT_OK(result);
}

TEST_F(UdsSocket, SendAndReceive) {  // NOLINT(readability-function-cognitive-complexity)
    // Arrange
    Socket server;
    Socket client;
    std::string path = GenerateString("UdsPath");
    ASSERT_OK(server.Create(AddressFamily::Uds));
    ASSERT_OK(server.Bind(path));
    ASSERT_OK(server.Listen());
    ASSERT_OK(client.Create(AddressFamily::Uds));
    ASSERT_OK(client.Connect(path));
    Socket acceptedClient;
    ASSERT_OK(server.Accept(acceptedClient));

    const uint32_t sendValue = GenerateU32();
    uint32_t receiveValue = 0;

    // Act
    SendExactly(client, &sendValue, sizeof(sendValue));
    ReceiveExactly(acceptedClient, &receiveValue, sizeof(receiveValue));

    // Assert
    ASSERT_EQ(sendValue, receiveValue);
}
