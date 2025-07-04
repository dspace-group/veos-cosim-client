// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "CoSimTypes.h"

namespace DsVeosCoSim {

struct CoSimServerConfig {
    uint16_t port{};
    bool enableRemoteAccess{};
    std::string serverName;
    bool isClientOptional{};
    bool startPortMapper{};
    bool registerAtPortMapper = true;
    SimulationTime stepSize{};
    SimulationCallback simulationStartedCallback;
    SimulationCallback simulationStoppedCallback;
    SimulationTerminatedCallback simulationTerminatedCallback;
    SimulationCallback simulationPausedCallback;
    SimulationCallback simulationContinuedCallback;
    CanMessageContainerReceivedCallback canMessageContainerReceivedCallback;
    LinMessageContainerReceivedCallback linMessageContainerReceivedCallback;
    EthMessageContainerReceivedCallback ethMessageContainerReceivedCallback;
    std::vector<IoSignalContainer> incomingSignals;
    std::vector<IoSignalContainer> outgoingSignals;
    std::vector<CanControllerContainer> canControllers;
    std::vector<EthControllerContainer> ethControllers;
    std::vector<LinControllerContainer> linControllers;
};

class CoSimServer {
protected:
    CoSimServer() = default;

public:
    virtual ~CoSimServer() = default;

    CoSimServer(const CoSimServer&) = delete;
    CoSimServer& operator=(const CoSimServer&) = delete;

    CoSimServer(CoSimServer&&) = delete;
    CoSimServer& operator=(CoSimServer&&) = delete;

    [[nodiscard]] virtual Result Load(const CoSimServerConfig& config) = 0;
    virtual void Unload() = 0;

    [[nodiscard]] virtual Result Start(SimulationTime simulationTime) = 0;
    [[nodiscard]] virtual Result Stop(SimulationTime simulationTime) = 0;
    [[nodiscard]] virtual Result Terminate(SimulationTime simulationTime, TerminateReason reason) = 0;
    [[nodiscard]] virtual Result Pause(SimulationTime simulationTime) = 0;
    [[nodiscard]] virtual Result Continue(SimulationTime simulationTime) = 0;
    [[nodiscard]] virtual Result Step(SimulationTime simulationTime, SimulationTime& nextSimulationTime) = 0;

    [[nodiscard]] virtual Result Write(IoSignalId signalId, uint32_t length, const void* value) const = 0;

    [[nodiscard]] virtual Result Read(IoSignalId signalId,
                                      uint32_t& length,
                                      const void** value,
                                      bool& valueRead) const = 0;

    [[nodiscard]] virtual Result Transmit(const CanMessage& message) const = 0;
    [[nodiscard]] virtual Result Transmit(const EthMessage& message) const = 0;
    [[nodiscard]] virtual Result Transmit(const LinMessage& message) const = 0;

    [[nodiscard]] virtual Result Transmit(const CanMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Transmit(const EthMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Transmit(const LinMessageContainer& messageContainer) const = 0;

    [[nodiscard]] virtual Result BackgroundService() = 0;

    [[nodiscard]] virtual Result GetLocalPort(uint16_t& port) const = 0;
};

[[nodiscard]] Result CreateServer(std::unique_ptr<CoSimServer>& server);

}  // namespace DsVeosCoSim
