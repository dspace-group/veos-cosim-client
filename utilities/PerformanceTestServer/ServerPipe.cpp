// Copyright dSPACE GmbH. All rights reserved.

#include <array>
#include <thread>

#include "LogHelper.h"
#include "OsAbstractionTestHelper.h"
#include "PerformanceTestHelper.h"

namespace {

void PipeServerRun() {
    try {
        LogTrace("Pipe server is listening on pipe {} ...", PipeName);

        std::array<char, BufferSize> buffer{};

        while (true) {
            Pipe pipe(PipeName);
            pipe.Accept();

            while (true) {
                if (!pipe.Read(buffer.data(), BufferSize)) {
                    break;
                }

                if (!pipe.Write(buffer.data(), BufferSize)) {
                    break;
                }
            }
        }
    } catch (const std::exception& e) {
        LogError("Exception in pipe server thread: {}", e.what());
    }
}

}  // namespace

void StartPipeServer() {
    std::thread(PipeServerRun).detach();
}
