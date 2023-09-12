// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimServerWrapper.h"

#include <chrono>

using namespace std::chrono;

namespace DsVeosCoSim {

CoSimServerWrapper::~CoSimServerWrapper() {
    (void)Unload();
}

Result CoSimServerWrapper::Load(const CoSimServerConfig& config) {
    CheckResult(_server.Load(config));

    _stopBackgroundThread = false;
    _backgroundThread = std::thread([this] {
        RunBackground();
    });
    return Result::Ok;
}

Result CoSimServerWrapper::Unload() {
    _stopBackgroundThread = true;
    _backgroundThread.join();

    return _server.Unload();
}

Result CoSimServerWrapper::Start(SimulationTime simulationTime) {
    std::lock_guard lock(_mutex);
    return _server.Start(simulationTime);
}

Result CoSimServerWrapper::Stop(SimulationTime simulationTime) {
    std::lock_guard lock(_mutex);
    return _server.Stop(simulationTime);
}

Result CoSimServerWrapper::Terminate(SimulationTime simulationTime, TerminateReason reason) {
    std::lock_guard lock(_mutex);
    return _server.Terminate(simulationTime, reason);
}

Result CoSimServerWrapper::Pause(SimulationTime simulationTime) {
    std::lock_guard lock(_mutex);
    return _server.Pause(simulationTime);
}

Result CoSimServerWrapper::Continue(SimulationTime simulationTime) {
    std::lock_guard lock(_mutex);
    return _server.Continue(simulationTime);
}

Result CoSimServerWrapper::Step(SimulationTime simulationTime, SimulationTime& nextSimulationTime) {
    std::lock_guard lock(_mutex);
    return _server.Step(simulationTime, nextSimulationTime);
}

Result CoSimServerWrapper::Write(IoSignalId signalId, uint32_t length, const void* value) {
    std::lock_guard lock(_mutex);
    return _server.Write(signalId, length, value);
}

Result CoSimServerWrapper::Transmit(const CanMessage& message) {
    std::lock_guard lock(_mutex);
    return _server.Transmit(message);
}

Result CoSimServerWrapper::Transmit(const LinMessage& message) {
    std::lock_guard lock(_mutex);
    return _server.Transmit(message);
}

Result CoSimServerWrapper::Transmit(const EthMessage& message) {
    std::lock_guard lock(_mutex);
    return _server.Transmit(message);
}

void CoSimServerWrapper::RunBackground() {
    while (!_stopBackgroundThread) {
        {
            std::lock_guard lock(_mutex);

            if (_server.BackgroundService() != Result::Ok) {
                throw std::runtime_error("Could not background.");
            }
        }

        std::this_thread::sleep_for(100ms);
    }
}

}  // namespace DsVeosCoSim
