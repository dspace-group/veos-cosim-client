// Copyright dSPACE GmbH. All rights reserved.

#include <fmt/format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Generator.h"
#include "LogHelper.h"
#include "PortMapper.h"

using namespace DsVeosCoSim;
using namespace testing;

namespace {

class TestPortMapper : public testing::Test {
protected:
    void SetUp() override {
        ClearLastMessage();
    }
};

TEST_F(TestPortMapper, StartOfServer) {
    // Arrange

    // Act and assert
    ASSERT_NO_THROW(PortMapperServer(false));
}

TEST_F(TestPortMapper, SetAndGet) {
    // Arrange
    PortMapperServer portMapperServer(false);

    const std::string serverName = GenerateString("Server名前");

    const uint16_t setPort = GenerateU16();

    uint16_t port{};

    // Act
    ASSERT_TRUE(PortMapper_SetPort(serverName, setPort));
    ASSERT_TRUE(PortMapper_GetPort("127.0.0.1", serverName, port));

    // Assert
    ASSERT_EQ(setPort, port);
}

#ifdef EXCEPTION_TESTS
TEST_F(TestPortMapper, GetWithoutSet) {
    // Arrange
    PortMapperServer portMapperServer(false);

    const std::string serverName = GenerateString("Server名前");

    uint16_t port{};

    // Act and assert
    ASSERT_THAT(
        [&]() {
            (void)PortMapper_GetPort("127.0.0.1", serverName, port);
        },
        ThrowsMessage<CoSimException>(
            fmt::format("Could not find port for dSPACE VEOS CoSim server '{}'.", serverName)));
}
#endif

#ifdef EXCEPTION_TESTS
TEST_F(TestPortMapper, GetAfterUnset) {
    // Arrange
    PortMapperServer portMapperServer(false);

    const std::string serverName = GenerateString("Server名前");

    const uint16_t setPort = GenerateU16();

    ASSERT_TRUE(PortMapper_SetPort(serverName, setPort));
    ASSERT_TRUE(PortMapper_UnsetPort(serverName));

    uint16_t port{};

    // Act and assert
    ASSERT_THAT(
        [&]() {
            (void)PortMapper_GetPort("127.0.0.1", serverName, port);
        },
        ThrowsMessage<CoSimException>(
            fmt::format("Could not find port for dSPACE VEOS CoSim server '{}'.", serverName)));
}
#endif

TEST_F(TestPortMapper, SetTwiceAndGet) {
    // Arrange
    PortMapperServer portMapperServer(false);

    const std::string serverName = GenerateString("Server名前");

    const uint16_t setPort1 = GenerateU16();
    const uint16_t setPort2 = setPort1 + 1;

    uint16_t port{};

    // Act
    EXPECT_TRUE(PortMapper_SetPort(serverName, setPort1));
    ASSERT_TRUE(PortMapper_SetPort(serverName, setPort2));
    ASSERT_TRUE(PortMapper_GetPort("127.0.0.1", serverName, port));

    // Assert
    ASSERT_EQ(setPort2, port);
}

}  // namespace
