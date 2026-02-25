// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "PerformanceTestClient.hpp"

#ifdef _WIN32

#include <array>
#include <cstdint>
#include <string>

#include "Error.hpp"
#include "Helper.hpp"
#include "OsUtilities.hpp"
#include "PerformanceTestHelper.hpp"

namespace DsVeosCoSim {

namespace {

[[nodiscard]] Result Run([[maybe_unused]] const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    ShmPipeClient client;
    CheckResult(ShmPipeClient::TryConnect(ShmPipeName, client));

    std::array<char, FrameSize> buffer{};

    connectedEvent.Set();

    while (!isStopped) {
        CheckResult(client.Send(buffer.data(), FrameSize));
        CheckResult(ReceiveComplete(client, buffer.data(), FrameSize));

        counter++;
    }

    return CreateOk();
}

void ShmPipeTest(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(Run(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run SHM Pipe Client.");
    }
}

}  // namespace

void RunShmPipeTest() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("SHM Pipe:");
    RunPerformanceTest(ShmPipeTest, "");
    LogTrace("");
}

}  // namespace DsVeosCoSim

#else

namespace DsVeosCoSim {

void RunShmPipeTest() {  // NOLINT(misc-use-internal-linkage)
}

}  // namespace DsVeosCoSim

#endif
