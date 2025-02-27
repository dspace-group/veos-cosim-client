// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <string>

#include "Generator.h"
#include "SharedMemory.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("SharedMemory名前\xF0\x9F\x98\x80");
}

class TestSharedMemory : public testing::Test {};

TEST_F(TestSharedMemory, CreateAndDestroy) {
    // Arrange
    const std::string name = GenerateName();

    // Act and assert
    ASSERT_NO_THROW((void)SharedMemory::CreateOrOpen(name, 100));
}

TEST_F(TestSharedMemory, ReadAndWriteOnSameSharedMemory) {
    // Arrange
    const std::string name = GenerateName();
    const SharedMemory sharedMemory = SharedMemory::CreateOrOpen(name, 100);

    auto* buffer = static_cast<uint8_t*>(sharedMemory.data());
    const uint32_t writeValue = GenerateU32();

    // Act
    *reinterpret_cast<uint32_t*>(buffer) = writeValue;
    const uint32_t readValue = *reinterpret_cast<uint32_t*>(buffer);

    // Assert
    ASSERT_EQ(writeValue, readValue);
}

TEST_F(TestSharedMemory, ReadAndWriteOnSharedMemories) {
    // Arrange
    const std::string name = GenerateName();
    const SharedMemory sharedMemory1 = SharedMemory::CreateOrOpen(name, 100);
    const SharedMemory sharedMemory2 = SharedMemory::CreateOrOpen(name, 100);

    auto* buffer1 = static_cast<uint8_t*>(sharedMemory1.data());
    auto* buffer2 = static_cast<uint8_t*>(sharedMemory2.data());
    const uint32_t writeValue = GenerateU32();

    // Act
    *reinterpret_cast<uint32_t*>(buffer1) = writeValue;
    const uint32_t readValue = *reinterpret_cast<uint32_t*>(buffer2);

    // Assert
    ASSERT_EQ(writeValue, readValue);
}

TEST_F(TestSharedMemory, CouldOpenExisting) {
    // Arrange
    const std::string name = GenerateName();
    const SharedMemory sharedMemory1 = SharedMemory::CreateOrOpen(name, 100);

    // Act and assert
    ASSERT_NO_THROW((void)SharedMemory::OpenExisting(name, 100));
}

TEST_F(TestSharedMemory, CouldNotOpenNonExisting) {
    // Arrange
    const std::string name = GenerateName();

    // Act and assert
    ASSERT_FALSE(SharedMemory::TryOpenExisting(name, 100));
}

}  // namespace

#endif
