// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <mutex>

#include "CoSimServer.h"
#include "CoSimTypes.h"

namespace DsVeosCoSim {

class CoSimServerWrapper final {
public:
    CoSimServerWrapper() = default;
    ~CoSimServerWrapper();

    CoSimServerWrapper(const CoSimServerWrapper&) = delete;
    CoSimServerWrapper& operator=(CoSimServerWrapper const&) = delete;

    CoSimServerWrapper(CoSimServerWrapper&&) = delete;
    CoSimServerWrapper& operator=(CoSimServerWrapper&&) = delete;

    [[nodiscard]] Result Load(const CoSimServerConfig& config);
    [[nodiscard]] Result Unload();

    // Commander functions
    [[nodiscard]] Result Start(SimulationTime simulationTime);
    [[nodiscard]] Result Stop(SimulationTime simulationTime);
    [[nodiscard]] Result Terminate(SimulationTime simulationTime, TerminateReason reason);
    [[nodiscard]] Result Pause(SimulationTime simulationTime);
    [[nodiscard]] Result Continue(SimulationTime simulationTime);
    [[nodiscard]] Result Step(SimulationTime simulationTime, SimulationTime& nextSimulationTime);

    // Data functions
    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value);
    [[nodiscard]] Result Transmit(const DsVeosCoSim_CanMessage& message);
    [[nodiscard]] Result Transmit(const DsVeosCoSim_LinMessage& message);
    [[nodiscard]] Result Transmit(const DsVeosCoSim_EthMessage& message);

    [[nodiscard]] uint16_t GetLocalPort() const;

private:
    void RunBackground();

    CoSimServer _server;

    bool _stopBackgroundThread{};
    std::thread _backgroundThread;

    std::recursive_mutex _mutex;
};

}  // namespace DsVeosCoSim
