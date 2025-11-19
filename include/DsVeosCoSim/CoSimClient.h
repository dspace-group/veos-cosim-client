// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "CoSimTypes.h"

namespace DsVeosCoSim {

class CoSimClient {
protected:
    CoSimClient() = default;

public:
    virtual ~CoSimClient() = default;

    CoSimClient(const CoSimClient&) = delete;
    CoSimClient& operator=(const CoSimClient&) = delete;

    CoSimClient(CoSimClient&&) = delete;
    CoSimClient& operator=(CoSimClient&&) = delete;

    [[nodiscard]] virtual Result Connect(const ConnectConfig& connectConfig) = 0;
    virtual void Disconnect() = 0;
    [[nodiscard]] virtual Result GetConnectionState(ConnectionState& connectionState) const = 0;

    [[nodiscard]] virtual Result GetStepSize(SimulationTime& stepSize) const = 0;
    [[nodiscard]] virtual Result GetCurrentSimulationTime(SimulationTime& simulationTime) const = 0;
    [[nodiscard]] virtual Result GetSimulationState(SimulationState& simulationState) const = 0;

    [[nodiscard]] virtual Result RunCallbackBasedCoSimulation(const Callbacks& callbacks) = 0;
    [[nodiscard]] virtual Result StartPollingBasedCoSimulation(const Callbacks& callbacks) = 0;
    [[nodiscard]] virtual Result PollCommand(SimulationTime& simulationTime, Command& command) = 0;
    [[nodiscard]] virtual Result FinishCommand() = 0;
    [[nodiscard]] virtual Result SetNextSimulationTime(SimulationTime simulationTime) = 0;

    [[nodiscard]] virtual Result Start() = 0;
    [[nodiscard]] virtual Result Stop() = 0;
    [[nodiscard]] virtual Result Terminate(TerminateReason terminateReason) = 0;
    [[nodiscard]] virtual Result Pause() = 0;
    [[nodiscard]] virtual Result Continue() = 0;

    [[nodiscard]] virtual Result GetIncomingSignals(uint32_t& signalsCount, const IoSignal*& signals) const = 0;
    [[nodiscard]] virtual Result GetOutgoingSignals(uint32_t& signalsCount, const IoSignal*& signals) const = 0;

    [[nodiscard]] virtual Result GetIncomingSignals(std::vector<IoSignal>& signals) const = 0;
    [[nodiscard]] virtual Result GetOutgoingSignals(std::vector<IoSignal>& signals) const = 0;

    [[nodiscard]] virtual Result Write(IoSignalId outgoingSignalId, uint32_t length, const void* value) const = 0;

    [[nodiscard]] virtual Result Read(IoSignalId incomingSignalId, uint32_t& length, void* value) const = 0;
    [[nodiscard]] virtual Result Read(IoSignalId incomingSignalId, uint32_t& length, const void** value) const = 0;

    [[nodiscard]] virtual Result GetCanControllers(uint32_t& controllersCount,
                                                   const CanController*& controllers) const = 0;
    [[nodiscard]] virtual Result GetEthControllers(uint32_t& controllersCount,
                                                   const EthController*& controllers) const = 0;
    [[nodiscard]] virtual Result GetLinControllers(uint32_t& controllersCount,
                                                   const LinController*& controllers) const = 0;
    [[nodiscard]] virtual Result GetFrControllers(uint32_t& controllersCount,
                                                   const FrController*& controllers) const = 0;

    [[nodiscard]] virtual Result GetCanControllers(std::vector<CanController>& controllers) const = 0;
    [[nodiscard]] virtual Result GetEthControllers(std::vector<EthController>& controllers) const = 0;
    [[nodiscard]] virtual Result GetLinControllers(std::vector<LinController>& controllers) const = 0;
    [[nodiscard]] virtual Result GetFrControllers(std::vector<FrController>& controllers) const = 0;

    [[nodiscard]] virtual Result Transmit(const CanMessage& message) const = 0;
    [[nodiscard]] virtual Result Transmit(const EthMessage& message) const = 0;
    [[nodiscard]] virtual Result Transmit(const LinMessage& message) const = 0;
    [[nodiscard]] virtual Result Transmit(const FrMessage& message) const = 0;



    [[nodiscard]] virtual Result Transmit(const CanMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Transmit(const EthMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Transmit(const LinMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Transmit(const FrMessageContainer& messageContainer) const = 0;

    [[nodiscard]] virtual Result Receive(CanMessage& message) const = 0;
    [[nodiscard]] virtual Result Receive(EthMessage& message) const = 0;
    [[nodiscard]] virtual Result Receive(LinMessage& message) const = 0;
    [[nodiscard]] virtual Result Receive(FrMessage& message) const = 0;

    [[nodiscard]] virtual Result Receive(CanMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Receive(EthMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Receive(LinMessageContainer& messageContainer) const = 0;
    [[nodiscard]] virtual Result Receive(FrMessageContainer& messageContainer) const = 0;
};

[[nodiscard]] Result CreateClient(std::unique_ptr<CoSimClient>& client);

}  // namespace DsVeosCoSim
