// Copyright dSPACE SE & Co. KG. All rights reserved.

#ifdef _WIN32

#include <array>
#include <cstdint>
#include <string>

#include "Helper.hpp"
#include "Logger.hpp"
#include "OsUtilities.hpp"
#include "PerformanceTestClient.hpp"
#include "PerformanceTestHelper.hpp"
#include "Result.hpp"

using namespace DsVeosCoSim;

namespace {

[[nodiscard]] Result RunClientShmPipeInternal([[maybe_unused]] const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
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

void RunClientShmPipe(const std::string& host, Event& connectedEvent, uint64_t& counter, const bool& isStopped) {
    if (!IsOk(RunClientShmPipeInternal(host, connectedEvent, counter, isStopped))) {
        LogError("Could not run SHM Pipe Client.");
    }
}

}  // namespace

void ClientShmPipe() {  // NOLINT(misc-use-internal-linkage)
    LogTrace("SHM Pipe:");
    RunPerformanceTest(RunClientShmPipe, "");
    LogTrace("");
}

#endif
