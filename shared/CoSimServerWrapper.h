// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <mutex>

#include "CoSimServer.h"
#include "CoSimTypes.h"

class CoSimServerWrapper final {
public:
    CoSimServerWrapper();
    ~CoSimServerWrapper();

    CoSimServerWrapper(const CoSimServerWrapper&) = delete;
    CoSimServerWrapper& operator=(CoSimServerWrapper const&) = delete;

    CoSimServerWrapper(CoSimServerWrapper&&) = delete;
    CoSimServerWrapper& operator=(CoSimServerWrapper&&) = delete;

    void Load(const DsVeosCoSim::CoSimServerConfig& config);
    void Unload();

    void Start(DsVeosCoSim::SimulationTime simulationTime);
    void Stop(DsVeosCoSim::SimulationTime simulationTime);
    void Terminate(DsVeosCoSim::SimulationTime simulationTime, DsVeosCoSim::TerminateReason reason);
    void Pause(DsVeosCoSim::SimulationTime simulationTime);
    void Continue(DsVeosCoSim::SimulationTime simulationTime);
    void Step(DsVeosCoSim::SimulationTime simulationTime, DsVeosCoSim::SimulationTime& nextSimulationTime);

    void Write(DsVeosCoSim::IoSignalId signalId, uint32_t length, const void* value);

    [[nodiscard]] bool Transmit(const DsVeosCoSim_CanMessage& message);
    [[nodiscard]] bool Transmit(const DsVeosCoSim_EthMessage& message);
    [[nodiscard]] bool Transmit(const DsVeosCoSim_LinMessage& message);

    [[nodiscard]] uint16_t GetLocalPort() const;

private:
    void RunBackground();

    DsVeosCoSim::CoSimServer _server;

    bool _stopBackgroundThread{};
    std::thread _backgroundThread;

    std::recursive_mutex _mutex;
};
