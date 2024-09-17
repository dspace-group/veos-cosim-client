// Copyright dSPACE GmbH. All rights reserved.

#include <thread>

#include "Logger.h"
#include "OsAbstractionTestHelper.h"
#include "PerformanceTestHelper.h"

using namespace DsVeosCoSim;

namespace {

void PipeServerRun() {
    try {
        LogTrace("Pipe server is listening on pipe {} ...", PipeName);

        char buffer[BufferSize]{};

        while (true) {
            Pipe pipe(PipeName);
            pipe.Accept();

            while (true) {
                if (!pipe.Read(buffer, BufferSize)) {
                    break;
                }

                if (!pipe.Write(buffer, BufferSize)) {
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