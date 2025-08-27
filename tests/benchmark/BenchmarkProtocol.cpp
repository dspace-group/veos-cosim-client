// Copyright dSPACE GmbH. All rights reserved.

#include <benchmark/benchmark.h>

#include <string>

#include "Channel.h"
#include "DsVeosCoSim/CoSimTypes.h"
#include "Helper.h"
#include "OsUtilities.h"
#include "Protocol.h"

using namespace std::chrono;
using namespace DsVeosCoSim;

namespace {

class MyChannelWriter : public ChannelWriter {
public:
    MyChannelWriter() {
        MustBeOk(SharedMemory::CreateOrOpen("My Test", 65536, _sharedMemory));
    }

    [[nodiscard]] Result Reserve(size_t size, BlockWriter& blockWriter) override {
        blockWriter = BlockWriter(reinterpret_cast<uint8_t*>(_sharedMemory.GetData()), size);
        return Result::Ok;
    }

    [[nodiscard]] Result Write(uint16_t value) override {
        *(reinterpret_cast<decltype(value)*>(_sharedMemory.GetData())) = value;
        return Result::Ok;
    }

    [[nodiscard]] Result Write(uint32_t value) override {
        *(reinterpret_cast<decltype(value)*>(_sharedMemory.GetData())) = value;
        return Result::Ok;
    }

    [[nodiscard]] Result Write(uint64_t value) override {
        *(reinterpret_cast<decltype(value)*>(_sharedMemory.GetData())) = value;
        return Result::Ok;
    }

    [[nodiscard]] Result Write(const void* source, size_t size) override {
        (void)memcpy(_sharedMemory.GetData(), source, size);
        return Result::Ok;
    }

    Result EndWrite() override {
        return Result::Ok;
    }

private:
    SharedMemory _sharedMemory;
};

void Write(benchmark::State& state) {
    MyChannelWriter writer;

    CanMessageContainer canMessageContainer;
    canMessageContainer.timestamp = SimulationTime(123456789);
    canMessageContainer.controllerId = static_cast<BusControllerId>(42);
    canMessageContainer.id = static_cast<BusMessageId>(43);
    canMessageContainer.flags = CanMessageFlags::FlexibleDataRateFormat;
    canMessageContainer.length = 44;

    for (auto _ : state) {
        MustBeOk(Protocol::WriteMessage(writer, canMessageContainer));
    }
}

}  // namespace

BENCHMARK(Write);
