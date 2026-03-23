// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include <string>

#include <gtest/gtest.h>

#include "Helper.hpp"
#include "OsUtilities.hpp"
#include "TestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("SharedMemory名前\xF0\x9F\x98\x80");
}

class TestSharedMemory : public testing::Test {};

TEST_F(TestSharedMemory, CreateShouldWork) {
    // Arrange
    std::string name = GenerateName();

    SharedMemory sharedMemory;

    // Act
    Result result = SharedMemory::CreateOrOpen(name, 100, sharedMemory);

    // Assert
    AssertOk(result);
}

TEST_F(TestSharedMemory, ReadAndWriteOnSameSharedMemory) {
    // Arrange
    std::string name = GenerateName();

    SharedMemory sharedMemory;
    AssertOk(SharedMemory::CreateOrOpen(name, 100, sharedMemory));

    auto* buffer = sharedMemory.GetData();
    uint32_t writeValue = GenerateU32();

    // Act
    *reinterpret_cast<uint32_t*>(buffer) = writeValue;
    uint32_t readValue = *reinterpret_cast<uint32_t*>(buffer);

    // Assert
    ASSERT_EQ(writeValue, readValue);
}

TEST_F(TestSharedMemory, ReadAndWriteOnSharedMemories) {
    // Arrange
    std::string name = GenerateName();

    SharedMemory sharedMemory1;
    AssertOk(SharedMemory::CreateOrOpen(name, 100, sharedMemory1));

    SharedMemory sharedMemory2;
    AssertOk(SharedMemory::CreateOrOpen(name, 100, sharedMemory2));

    auto* buffer1 = sharedMemory1.GetData();
    auto* buffer2 = sharedMemory2.GetData();
    uint32_t writeValue = GenerateU32();

    // Act
    *reinterpret_cast<uint32_t*>(buffer1) = writeValue;
    uint32_t readValue = *reinterpret_cast<uint32_t*>(buffer2);

    // Assert
    ASSERT_EQ(writeValue, readValue);
}

TEST_F(TestSharedMemory, CouldOpenExisting) {
    // Arrange
    std::string name = GenerateName();

    SharedMemory sharedMemory1;
    AssertOk(SharedMemory::CreateOrOpen(name, 100, sharedMemory1));

    SharedMemory sharedMemory2;

    // Act
    Result result = SharedMemory::TryOpenExisting(name, 100, sharedMemory2);

    // Assert
    AssertOk(result);
}

TEST_F(TestSharedMemory, CouldNotOpenNonExisting) {
    // Arrange
    std::string name = GenerateName();

    SharedMemory sharedMemory;

    // Act
    Result result = SharedMemory::TryOpenExisting(name, 100, sharedMemory);

    // Assert
    AssertNotConnected(result);
}

}  // namespace

#endif
