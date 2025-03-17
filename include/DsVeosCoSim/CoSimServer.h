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
    CanMessageReceivedCallback canMessageReceivedCallback;
    LinMessageReceivedCallback linMessageReceivedCallback;
    EthMessageReceivedCallback ethMessageReceivedCallback;
    std::vector<IoSignalContainer> incomingSignals;
    std::vector<IoSignalContainer> outgoingSignals;
    std::vector<CanControllerContainer> canControllers;
    std::vector<EthControllerContainer> ethControllers;
    std::vector<LinControllerContainer> linControllers;
};

class CoSimServer {  // NOLINT
public:
    virtual ~CoSimServer() noexcept = default;

    virtual void Load(const CoSimServerConfig& config) = 0;
    virtual void Unload() = 0;

    virtual void Start(SimulationTime simulationTime) = 0;
    virtual void Stop(SimulationTime simulationTime) = 0;
    virtual void Terminate(SimulationTime simulationTime, TerminateReason reason) = 0;
    virtual void Pause(SimulationTime simulationTime) = 0;
    virtual void Continue(SimulationTime simulationTime) = 0;
    virtual SimulationTime Step(SimulationTime simulationTime) = 0;

    virtual void Write(IoSignalId signalId, uint32_t length, const void* value) const = 0;

    virtual void Read(IoSignalId signalId, uint32_t& length, const void** value) const = 0;

    [[nodiscard]] virtual bool Transmit(const CanMessage& message) const = 0;
    [[nodiscard]] virtual bool Transmit(const EthMessage& message) const = 0;
    [[nodiscard]] virtual bool Transmit(const LinMessage& message) const = 0;

    virtual void BackgroundService() = 0;

    [[nodiscard]] virtual uint16_t GetLocalPort() const = 0;
};

std::unique_ptr<CoSimServer> CreateServer();

}  // namespace DsVeosCoSim
