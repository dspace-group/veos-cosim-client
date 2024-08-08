// Copyright dSPACE GmbH. All rights reserved.

#include "Generator.h"
#include "Logger.h"
#include "PortMapper.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

class TestPortMapper : public testing::Test {
protected:
    void SetUp() override {
        SetLogCallback(OnLogCallback);
        ClearLastMessage();
    }
};

TEST_F(TestPortMapper, StartOfServer) {
    // Arrange
    PortMapperServer portMapperServer;

    // Act
    const Result result = portMapperServer.Start(false);

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestPortMapper, SetAndGet) {
    // Arrange
    PortMapperServer portMapperServer;
    ASSERT_OK(portMapperServer.Start(false));

    const std::string serverName = GenerateString("Server名前");

    const uint16_t setPort = GenerateU16();
    uint16_t getPort{};

    // Act
    ASSERT_OK(PortMapper_SetPort(serverName, setPort));
    ASSERT_OK(PortMapper_GetPort("127.0.0.1", serverName, getPort));

    // Assert
    ASSERT_EQ(setPort, getPort);
}

TEST_F(TestPortMapper, GetWithoutSet) {
    // Arrange
    PortMapperServer portMapperServer;
    ASSERT_OK(portMapperServer.Start(false));

    const std::string serverName = GenerateString("Server名前");

    // Act
    uint16_t port{};
    ASSERT_ERROR(PortMapper_GetPort("127.0.0.1", serverName, port));

    // Assert
    AssertLastMessage("Could not find port for dSPACE VEOS CoSim server '" + serverName + "'.");
}

TEST_F(TestPortMapper, GetAfterUnset) {
    // Arrange
    PortMapperServer portMapperServer;
    ASSERT_OK(portMapperServer.Start(false));

    const std::string serverName = GenerateString("Server名前");

    const uint16_t setPort = GenerateU16();
    uint16_t getPort{};

    // Act
    ASSERT_OK(PortMapper_SetPort(serverName, setPort));
    ASSERT_OK(PortMapper_UnsetPort(serverName));
    ASSERT_ERROR(PortMapper_GetPort("127.0.0.1", serverName, getPort));

    // Assert
    AssertLastMessage("Could not find port for dSPACE VEOS CoSim server '" + serverName + "'.");
}

TEST_F(TestPortMapper, SetTwiceAndGet) {
    // Arrange
    PortMapperServer portMapperServer;
    ASSERT_OK(portMapperServer.Start(false));

    const std::string serverName = GenerateString("Server名前");

    const uint16_t setPort1 = GenerateU16();
    const uint16_t setPort2 = setPort1 + 1;
    uint16_t getPort{};

    // Act
    ASSERT_OK(PortMapper_SetPort(serverName, setPort1));
    ASSERT_OK(PortMapper_SetPort(serverName, setPort2));
    ASSERT_OK(PortMapper_GetPort("127.0.0.1", serverName, getPort));

    // Assert
    ASSERT_EQ(setPort2, getPort);
}
