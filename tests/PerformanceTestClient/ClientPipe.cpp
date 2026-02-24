// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestClient.hpp"

#include <array>
#include <cstdint>
#include <string>

#include "Error.hpp"
#include "Helper.hpp"
#include "OsAbstractionTestHelper.hpp"
#include "PerformanceTestHelper.hpp"

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
