// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "Generator.h"
#include "OsUtilities.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("SharedMemory名前\xF0\x9F\x98\x80");
}

class TestSharedMemory : public testing::Test {};

TEST_F(TestSharedMemory, CreateAndDestroy) {
    // Arrange
    const std::string name = GenerateName();

    // Act
    const std::unique_ptr<SharedMemory> sharedMemory = CreateOrOpenSharedMemory(name, 100);

    // Assert
    ASSERT_TRUE(sharedMemory);
}

TEST_F(TestSharedMemory, ReadAndWriteOnSameSharedMemory) {
    // Arrange
    const std::string name = GenerateName();
    const std::unique_ptr<SharedMemory> sharedMemory = CreateOrOpenSharedMemory(name, 100);

    auto* buffer = static_cast<uint8_t*>(sharedMemory->data());
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
    const std::unique_ptr<SharedMemory> sharedMemory1 = CreateOrOpenSharedMemory(name, 100);
    const std::unique_ptr<SharedMemory> sharedMemory2 = CreateOrOpenSharedMemory(name, 100);

    auto* buffer1 = static_cast<uint8_t*>(sharedMemory1->data());
    auto* buffer2 = static_cast<uint8_t*>(sharedMemory2->data());
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
    const std::unique_ptr<SharedMemory> sharedMemory1 = CreateOrOpenSharedMemory(name, 100);

    // Act
    const std::unique_ptr<SharedMemory> sharedMemory2 = TryOpenExistingSharedMemory(name, 100);

    // Assert
    ASSERT_TRUE(sharedMemory2);
}

TEST_F(TestSharedMemory, CouldNotOpenNonExisting) {
    // Arrange
    const std::string name = GenerateName();

    // Act
    const std::unique_ptr<SharedMemory> sharedMemory2 = TryOpenExistingSharedMemory(name, 100);

    // Assert
    ASSERT_FALSE(sharedMemory2);
}

}  // namespace

#endif
