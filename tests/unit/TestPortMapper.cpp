// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <cstdint>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <PortMapper.hpp>
#include <Result.hpp>

#include "Helper.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;
using namespace testing;

namespace {

class TestPortMapper : public Test {
protected:
    void SetUp() override {
        AssertOk(CreatePortMapperServer(true, _portMapperServer));
    }

    std::unique_ptr<PortMapperServer> _portMapperServer;
};

TEST_F(TestPortMapper, SetPort) {
    // Arrange
    std::string serverName = GenerateString("Server名前");
    uint16_t port = GenerateU16();

    // Act
    Result result = PortMapperSetPort(serverName, port);

    // Assert
    AssertOk(result);
    uint16_t retrievedPort{};
    AssertOk(PortMapperGetPort("127.0.0.1", serverName, retrievedPort));
    EXPECT_EQ(port, retrievedPort);
}

TEST_F(TestPortMapper, UnsetPortOfExistingPort) {
    // Arrange
    std::string serverName = GenerateString("Server名前");
    uint16_t port = GenerateU16();
    AssertOk(PortMapperSetPort(serverName, port));

    // Act
    Result result = PortMapperUnsetPort(serverName);

    // Assert
    AssertOk(result);
    uint16_t retrievedPort{};
    AssertError(PortMapperGetPort("127.0.0.1", serverName, retrievedPort));
}

TEST_F(TestPortMapper, UnsetPortOfNonExistingPort) {
    // Arrange
    std::string serverName = GenerateString("Server名前");
    std::string otherServerName = GenerateString("OtherServer名前");
    uint16_t otherPort = GenerateU16();
    AssertOk(PortMapperSetPort(otherServerName, otherPort));

    // Act — PortMapperServer succeeds silently for non-existing entries
    Result result = PortMapperUnsetPort(serverName);

    // Assert
    AssertOk(result);
    uint16_t retrievedPort{};
    AssertOk(PortMapperGetPort("127.0.0.1", otherServerName, retrievedPort));
    EXPECT_EQ(otherPort, retrievedPort);
}

TEST_F(TestPortMapper, GetPort) {
    // Arrange
    std::string serverName = GenerateString("Server名前");
    uint16_t expectedPort = GenerateU16();
    AssertOk(PortMapperSetPort(serverName, expectedPort));

    // Act
    uint16_t port{};
    Result result = PortMapperGetPort("127.0.0.1", serverName, port);

    // Assert
    AssertOk(result);
    EXPECT_EQ(expectedPort, port);
}

TEST_F(TestPortMapper, GetPortOfNonExistingServer) {
    // Arrange
    std::string serverName = GenerateString("Server名前");
    std::string otherServerName = GenerateString("OtherServer名前");
    uint16_t otherPort = GenerateU16();
    AssertOk(PortMapperSetPort(otherServerName, otherPort));

    // Act
    uint16_t port{};
    Result result = PortMapperGetPort("127.0.0.1", serverName, port);

    // Assert
    AssertError(result);
}

}  // namespace
