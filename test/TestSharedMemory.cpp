// Copyright dSPACE GmbH. All rights reserved.

#include "SharedMemory.h"
#include "Generator.h"
#include "Logger.h"
#include "TestHelper.h"

using namespace DsVeosCoSim;

class TestSharedMemory : public testing::Test {
protected:
    void SetUp() override {
        SetLogCallback(OnLogCallback);
    }
};

TEST_F(TestSharedMemory, CreateAndDestroy) {
    // Arrange
    SharedMemory sharedMemory(GenerateString("SharedMemory"));

    // Act
    Result result = sharedMemory.Create();

    // Assert
    ASSERT_OK(result);
}

TEST_F(TestSharedMemory, ReadAndWriteOnSameSharedMemory) {
    // Arrange
    std::string name = GenerateString("SharedMemory");
    SharedMemory sharedMemory(name);
    ASSERT_OK(sharedMemory.Create());

    uint8_t* buffer = (uint8_t*)sharedMemory.GetBuffer();
    uint32_t writeValue = GenerateU32();

    // Act
    *((uint32_t*)buffer) = writeValue;
    uint32_t readValue = *((uint32_t*)buffer);

    // Assert
    ASSERT_EQ(writeValue, readValue);
}

TEST_F(TestSharedMemory, ReadAndWriteOnSharedMemories) {
    // Arrange
    std::string name = GenerateString("SharedMemory");
    SharedMemory sharedMemory1(name);
    SharedMemory sharedMemory2(name);
    ASSERT_OK(sharedMemory1.Create());
    ASSERT_OK(sharedMemory2.Create());

    uint8_t* buffer1 = (uint8_t*)sharedMemory1.GetBuffer();
    uint8_t* buffer2 = (uint8_t*)sharedMemory2.GetBuffer();
    uint32_t writeValue = GenerateU32();

    // Act
    *((uint32_t*)buffer1) = writeValue;
    uint32_t readValue = *((uint32_t*)buffer2);

    // Assert
    ASSERT_EQ(writeValue, readValue);
}
