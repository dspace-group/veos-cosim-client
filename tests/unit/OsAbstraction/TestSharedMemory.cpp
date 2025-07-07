// Copyright dSPACE GmbH. All rights reserved.

#ifdef _WIN32

#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "Helper.h"
#include "OsUtilities.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] std::string GenerateName() {
    return GenerateString("SharedMemory名前\xF0\x9F\x98\x80");
}

class TestSharedMemory : public testing::Test {};

TEST_F(TestSharedMemory, ReadAndWriteOnSameSharedMemory) {
    // Arrange
    std::string name = GenerateName();
    SharedMemory sharedMemory;
    ExpectOk(SharedMemory::CreateOrOpen(name, 100, sharedMemory));

    auto* buffer = static_cast<uint8_t*>(sharedMemory.GetData());
    uint32_t writeValue = GenerateU32();

    // Act
    *reinterpret_cast<uint32_t*>(buffer) = writeValue;
    uint32_t readValue = *reinterpret_cast<uint32_t*>(buffer);

    // Assert
    AssertEq(writeValue, readValue);
}

TEST_F(TestSharedMemory, ReadAndWriteOnSharedMemories) {
    // Arrange
    std::string name = GenerateName();
    SharedMemory sharedMemory1;
    ExpectOk(SharedMemory::CreateOrOpen(name, 100, sharedMemory1));
    SharedMemory sharedMemory2;
    ExpectOk(SharedMemory::CreateOrOpen(name, 100, sharedMemory2));

    auto* buffer1 = static_cast<uint8_t*>(sharedMemory1.GetData());
    auto* buffer2 = static_cast<uint8_t*>(sharedMemory2.GetData());
    uint32_t writeValue = GenerateU32();

    // Act
    *reinterpret_cast<uint32_t*>(buffer1) = writeValue;
    uint32_t readValue = *reinterpret_cast<uint32_t*>(buffer2);

    // Assert
    AssertEq(writeValue, readValue);
}

TEST_F(TestSharedMemory, CouldOpenExisting) {
    // Arrange
    std::string name = GenerateName();
    SharedMemory sharedMemory1;
    ExpectOk(SharedMemory::CreateOrOpen(name, 100, sharedMemory1));

    std::optional<SharedMemory> sharedMemory2;

    // Act
    AssertOk(SharedMemory::TryOpenExisting(name, 100, sharedMemory2));

    // Assert
    AssertTrue(sharedMemory2);
}

TEST_F(TestSharedMemory, CouldNotOpenNonExisting) {
    // Arrange
    std::string name = GenerateName();

    std::optional<SharedMemory> sharedMemory2;

    // Act
    AssertOk(SharedMemory::TryOpenExisting(name, 100, sharedMemory2));

    // Assert
    AssertFalse(sharedMemory2);
}

}  // namespace

#endif
