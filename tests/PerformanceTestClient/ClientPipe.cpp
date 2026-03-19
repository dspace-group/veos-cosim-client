// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestClient.hpp"

#include <array>
#include <cstdint>
#include <string>

#include "Helper.hpp"
#include "OsAbstractionTestHelper.hpp"
#include "PerformanceTestHelper.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result Run([[maybe_unused]] const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    Pipe pipe;
    CheckResult(pipe.Initialize(PipeName));
    CheckResult(pipe.Connect());

    std::array<char, BufferSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(pipe.Write(buffer.data(), BufferSize));

        CheckResult(pipe.Read(buffer.data(), BufferSize));

        counter++;
    }

    return Result::Ok;
}

void PipeClientRun(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run pipe client.");
    }
}

}  // namespace

void RunPipeTest() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("Pipes:");
    RunPerformanceTest(PipeClientRun, "");
    LogTrace("");
}
