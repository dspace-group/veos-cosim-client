// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "CoSimTypes.h"

namespace DsVeosCoSim {

void SetLogCallback(LogCallback logCallback);

class CoSimClient {  // NOLINT
public:
    virtual ~CoSimClient() noexcept = default;

    [[nodiscard]] virtual bool Connect(const ConnectConfig& connectConfig) = 0;
    virtual void Disconnect() = 0;
    [[nodiscard]] virtual ConnectionState GetConnectionState() const = 0;

    [[nodiscard]] virtual SimulationTime GetStepSize() const = 0;

    [[nodiscard]] virtual bool RunCallbackBasedCoSimulation(const Callbacks& callbacks) = 0;
    virtual void StartPollingBasedCoSimulation(const Callbacks& callbacks) = 0;
    [[nodiscard]] virtual bool PollCommand(SimulationTime& simulationTime, Command& command, bool returnOnPing) = 0;
    [[nodiscard]] virtual bool FinishCommand() = 0;
    virtual void SetNextSimulationTime(SimulationTime simulationTime) = 0;

    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Terminate(TerminateReason terminateReason) = 0;
    virtual void Pause() = 0;
    virtual void Continue() = 0;

    virtual void GetIncomingSignals(uint32_t* incomingSignalsCount, const IoSignal** incomingSignals) const = 0;
    virtual void GetOutgoingSignals(uint32_t* outgoingSignalsCount, const IoSignal** outgoingSignals) const = 0;

    [[nodiscard]] virtual const std::vector<IoSignal>& GetIncomingSignals() const = 0;
    [[nodiscard]] virtual const std::vector<IoSignal>& GetOutgoingSignals() const = 0;

    virtual void Write(IoSignalId outgoingSignalId, uint32_t length, const void* value) const = 0;

    virtual void Read(IoSignalId incomingSignalId, uint32_t& length, void* value) const = 0;
    virtual void Read(IoSignalId incomingSignalId, uint32_t& length, const void** value) const = 0;

    virtual void GetCanControllers(uint32_t* controllersCount, const CanController** controllers) const = 0;
    virtual void GetEthControllers(uint32_t* controllersCount, const EthController** controllers) const = 0;
    virtual void GetLinControllers(uint32_t* controllersCount, const LinController** controllers) const = 0;

    [[nodiscard]] virtual const std::vector<CanController>& GetCanControllers() const = 0;
    [[nodiscard]] virtual const std::vector<EthController>& GetEthControllers() const = 0;
    [[nodiscard]] virtual const std::vector<LinController>& GetLinControllers() const = 0;

    [[nodiscard]] virtual bool Transmit(const CanMessage& message) const = 0;
    [[nodiscard]] virtual bool Transmit(const EthMessage& message) const = 0;
    [[nodiscard]] virtual bool Transmit(const LinMessage& message) const = 0;

    [[nodiscard]] virtual bool Receive(CanMessage& message) const = 0;
    [[nodiscard]] virtual bool Receive(EthMessage& message) const = 0;
    [[nodiscard]] virtual bool Receive(LinMessage& message) const = 0;
};

std::unique_ptr<CoSimClient> CreateClient();

}  // namespace DsVeosCoSim
