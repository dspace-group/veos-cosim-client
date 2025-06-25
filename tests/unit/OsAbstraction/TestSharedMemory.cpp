// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "Generator.h"
#include "OsUtilities.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("SharedMemory名前\xF0\x9F\x98\x80");
}

class TestSharedMemory : public testing::Test {};

TEST_F(TestSharedMemory, ReadAndWriteOnSameSharedMemory) {
    // Arrange
    std::string name = GenerateName();
    SharedMemory sharedMemory = SharedMemory::CreateOrOpen(name, 100);

    auto* buffer = static_cast<uint8_t*>(sharedMemory.data());
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
    SharedMemory sharedMemory1 = SharedMemory::CreateOrOpen(name, 100);
    SharedMemory sharedMemory2 = SharedMemory::CreateOrOpen(name, 100);

    auto* buffer1 = static_cast<uint8_t*>(sharedMemory1.data());
    auto* buffer2 = static_cast<uint8_t*>(sharedMemory2.data());
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
    SharedMemory sharedMemory1 = SharedMemory::CreateOrOpen(name, 100);

    // Act
    std::optional<SharedMemory> sharedMemory2 = SharedMemory::TryOpenExisting(name, 100);

    // Assert
    ASSERT_TRUE(sharedMemory2);
}

TEST_F(TestSharedMemory, CouldNotOpenNonExisting) {
    // Arrange
    std::string name = GenerateName();

    // Act
    std::optional<SharedMemory> sharedMemory2 = SharedMemory::TryOpenExisting(name, 100);

    // Assert
    ASSERT_FALSE(sharedMemory2);
}

}  // namespace

#endif
