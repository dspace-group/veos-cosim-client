// Copyright dSPACE SE & Co. KG. All rights reserved.

#include <array>
#include <cstdint>
#include <string>

#include "Logger.hpp"
#include "OsAbstractionTestHelper.hpp"
#include "PerformanceTestClient.hpp"
#include "PerformanceTestHelper.hpp"
#include "Result.hpp"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result Run([[maybe_unused]] const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    PipeClient client;
    CheckResult(PipeClient::Connect(PipeName, client));

    std::array<char, FrameSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(client.Write(buffer.data(), FrameSize));
        CheckResult(client.Read(buffer.data(), FrameSize));

        counter++;
    }

    return CreateOk();
}

void PipeTest(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run Pipe Client.");
    }
}

}  // namespace

void RunPipeTest() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Pipe:");
    RunPerformanceTest(PipeTest, "");
    LogTrace("");
}

}  // namespace DsVeosCoSim
