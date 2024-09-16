// Copyright dSPACE GmbH. All rights reserved.

#include "CoSimServerWrapper.h"

using namespace DsVeosCoSim;

CoSimServerWrapper::CoSimServerWrapper() {
    _stopBackgroundThread = false;
    _backgroundThread = std::thread([this] {
        RunBackground();
    });
}

CoSimServerWrapper::~CoSimServerWrapper() {
    _stopBackgroundThread = true;
    _backgroundThread.join();
}

void CoSimServerWrapper::Load(const CoSimServerConfig& config) {
    std::lock_guard lock(_mutex);
    _server.Load(config);
}

void CoSimServerWrapper::Unload() {
    std::lock_guard lock(_mutex);
    _server.Unload();
}

void CoSimServerWrapper::Start(SimulationTime simulationTime) {
    std::lock_guard lock(_mutex);
    _server.Start(simulationTime);
}

void CoSimServerWrapper::Stop(SimulationTime simulationTime) {
    std::lock_guard lock(_mutex);
    _server.Stop(simulationTime);
}

void CoSimServerWrapper::Terminate(SimulationTime simulationTime, TerminateReason reason) {
    std::lock_guard lock(_mutex);
    _server.Terminate(simulationTime, reason);
}

void CoSimServerWrapper::Pause(SimulationTime simulationTime) {
    std::lock_guard lock(_mutex);
    _server.Pause(simulationTime);
}

void CoSimServerWrapper::Continue(SimulationTime simulationTime) {
    std::lock_guard lock(_mutex);
    _server.Continue(simulationTime);
}

void CoSimServerWrapper::Step(SimulationTime simulationTime, SimulationTime& nextSimulationTime) {
    std::lock_guard lock(_mutex);
    _server.Step(simulationTime, nextSimulationTime);
}

void CoSimServerWrapper::Write(IoSignalId signalId, uint32_t length, const void* value) {
    std::lock_guard lock(_mutex);
    _server.Write(signalId, length, value);
}

bool CoSimServerWrapper::Transmit(const DsVeosCoSim_CanMessage& message) {
    std::lock_guard lock(_mutex);
    return _server.Transmit(message);
}

bool CoSimServerWrapper::Transmit(const DsVeosCoSim_EthMessage& message) {
    std::lock_guard lock(_mutex);
    return _server.Transmit(message);
}

bool CoSimServerWrapper::Transmit(const DsVeosCoSim_LinMessage& message) {
    std::lock_guard lock(_mutex);
    return _server.Transmit(message);
}

uint16_t CoSimServerWrapper::GetLocalPort() const {
    return _server.GetLocalPort();
}

void CoSimServerWrapper::RunBackground() {
    while (!_stopBackgroundThread) {
        {
            std::lock_guard lock(_mutex);
            _server.BackgroundService();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
