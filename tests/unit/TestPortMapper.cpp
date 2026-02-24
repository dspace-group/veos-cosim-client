// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <fmt/format.h>

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "Helper.hpp"
#include "PortMapper.hpp"
#include "TestHelper.hpp"

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
    Result result = CreatePortMapperServer(false, portMapperServer);

    // Assert
    AssertOk(result);
}

TEST_F(TestPortMapper, SetAndGet) {
    // Arrange
    std::unique_ptr<PortMapperServer> portMapperServer;
    AssertOk(CreatePortMapperServer(false, portMapperServer));

    std::string serverName = GenerateString("Server名前");

    uint16_t setPort = GenerateU16();

    uint16_t port{};

    // Act
    AssertOk(PortMapperSetPort(serverName, setPort));
    AssertOk(PortMapperGetPort("127.0.0.1", serverName, port));

    // Assert
    ASSERT_EQ(setPort, port);
}

TEST_F(TestPortMapper, SetTwiceAndGet) {
    // Arrange
    std::unique_ptr<PortMapperServer> portMapperServer;
    AssertOk(CreatePortMapperServer(false, portMapperServer));

    std::string serverName = GenerateString("Server名前");

    uint16_t setPort1 = GenerateU16();
    auto setPort2 = static_cast<uint16_t>(setPort1 + 1);

    uint16_t port{};

    // Act
    AssertOk(PortMapperSetPort(serverName, setPort1));
    AssertOk(PortMapperSetPort(serverName, setPort2));
    AssertOk(PortMapperGetPort("127.0.0.1", serverName, port));

    // Assert
    ASSERT_EQ(setPort2, port);
}

}  // namespace
