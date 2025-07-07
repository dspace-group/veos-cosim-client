// Copyright dSPACE GmbH. All rights reserved.

#include <fmt/format.h>

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "Helper.h"
#include "PortMapper.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;
using namespace testing;

namespace {

class TestPortMapper : public Test {
protected:
    void SetUp() override {
        ClearLastMessage();
    }
};

TEST_F(TestPortMapper, StartOfServer) {
    // Arrange
    std::unique_ptr<PortMapperServer> portMapperServer;

    // Act
    AssertOk(CreatePortMapperServer(false, portMapperServer));

    // Assert
    AssertTrue(portMapperServer);
}

TEST_F(TestPortMapper, SetAndGet) {
    // Arrange
    std::unique_ptr<PortMapperServer> portMapperServer;
    ExpectOk(CreatePortMapperServer(false, portMapperServer));
    ExpectTrue(portMapperServer);

    std::string serverName = GenerateString("Server名前");

    uint16_t setPort = GenerateU16();

    uint16_t port{};

    // Act
    AssertOk(PortMapperSetPort(serverName, setPort));
    AssertOk(PortMapperGetPort("127.0.0.1", serverName, port));

    // Assert
    AssertEq(setPort, port);
}

TEST_F(TestPortMapper, SetTwiceAndGet) {
    // Arrange
    std::unique_ptr<PortMapperServer> portMapperServer;
    ExpectOk(CreatePortMapperServer(false, portMapperServer));
    ExpectTrue(portMapperServer);

    std::string serverName = GenerateString("Server名前");

    uint16_t setPort1 = GenerateU16();
    uint16_t setPort2 = static_cast<uint16_t>(setPort1 + 1);

    uint16_t port{};

    // Act
    ExpectOk(PortMapperSetPort(serverName, setPort1));
    AssertOk(PortMapperSetPort(serverName, setPort2));
    AssertOk(PortMapperGetPort("127.0.0.1", serverName, port));

    // Assert
    AssertEq(setPort2, port);
}

}  // namespace
