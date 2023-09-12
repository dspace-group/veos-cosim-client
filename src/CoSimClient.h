// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <vector>

#include "CoSimTypes.h"
#include "Communication.h"
#include "Protocol.h"

namespace DsVeosCoSim {

enum class ResponderMode {
    Unknown = 0,
    Blocking = 1,
    NonBlocking
};

class CoSimClient final {
public:
    CoSimClient() = default;

    CoSimClient(const CoSimClient&) = delete;
    CoSimClient& operator=(CoSimClient const&) = delete;

    CoSimClient(CoSimClient&&) = delete;
    CoSimClient& operator=(CoSimClient&&) = delete;

    [[nodiscard]] Result Connect(const ConnectConfig& connectConfig);
    void Disconnect();
    [[nodiscard]] ConnectionState GetConnectionState() const;

    [[nodiscard]] Result RunCallbackBasedCoSimulation(const Callbacks& callbacks);
    [[nodiscard]] Result StartPollingBasedCoSimulation(const Callbacks& callbacks);
    [[nodiscard]] Result PollCommand(SimulationTime& simulationTime, Command& command);
    [[nodiscard]] Result FinishCommand();

    [[nodiscard]] Result SetNextSimulationTime(SimulationTime simulationTime);

    [[nodiscard]] Result GetIncomingSignals(uint32_t* incomingSignalsCount, const IoSignal** incomingSignals) const;
    [[nodiscard]] Result GetIncomingSignals(std::vector<IoSignal>& incomingSignals) const;

    [[nodiscard]] Result GetOutgoingSignals(uint32_t* outgoingSignalsCount, const IoSignal** outgoingSignals) const;
    [[nodiscard]] Result GetOutgoingSignals(std::vector<IoSignal>& outgoingSignals) const;

    [[nodiscard]] Result Read(IoSignalId incomingSignalId, uint32_t& length, void* value);
    [[nodiscard]] Result Write(IoSignalId outgoingSignalId, uint32_t length, const void* value);

    [[nodiscard]] Result GetControllers(uint32_t* controllersCount, const CanController** controllers) const;
    [[nodiscard]] Result GetControllers(std::vector<CanController>& controllers) const;

    [[nodiscard]] Result Receive(CanMessage& message);
    [[nodiscard]] Result Transmit(const CanMessage& message);

    [[nodiscard]] Result GetControllers(uint32_t* controllersCount, const EthController** controllers) const;
    [[nodiscard]] Result GetControllers(std::vector<EthController>& controllers) const;

    [[nodiscard]] Result Receive(EthMessage& message);
    [[nodiscard]] Result Transmit(const EthMessage& message);

    [[nodiscard]] Result GetControllers(uint32_t* controllersCount, const LinController** controllers) const;
    [[nodiscard]] Result GetControllers(std::vector<LinController>& controllers) const;

    [[nodiscard]] Result Receive(LinMessage& message);
    [[nodiscard]] Result Transmit(const LinMessage& message);

private:
    void SetCallbacks(const Callbacks& callbacks);
    void ResetDataFromPreviousConnect();
    void CloseConnection();

    [[nodiscard]] Result SendConnectRequest();
    [[nodiscard]] Result ConnectOnAccepted();
    [[nodiscard]] Result ConnectOnDeclined();
    [[nodiscard]] Result ReceiveConnectResponse();

    [[nodiscard]] Result RunCallbackBasedCoSimulationInternal();
    [[nodiscard]] Result PollCommandInternal(SimulationTime& simulationTime, Command& command);
    [[nodiscard]] Result FinishCommandInternal();

    [[nodiscard]] Result OnStep();
    [[nodiscard]] Result OnStart();
    [[nodiscard]] Result OnStop();
    [[nodiscard]] Result OnTerminate();
    [[nodiscard]] Result OnPause();
    [[nodiscard]] Result OnContinue();

    [[nodiscard]] Result WaitForNextFrame(FrameKind& frameKind);
    [[nodiscard]] Result WaitForOkFrame();

    [[nodiscard]] Result EnsureIsConnected() const;
    [[nodiscard]] Result EnsureIsInResponderModeBlocking();
    [[nodiscard]] Result EnsureIsInResponderModeNonBlocking();

    Channel _channel;

    bool _isConnected{};
    Callbacks _callbacks{};
    SimulationTime _currentSimulationTime{};
    SimulationTime _nextSimulationTime{};

    std::string _remoteIpAddress;
    std::string _serverName;
    std::string _clientName;
    uint16_t _remotePort{};

    ResponderMode _responderMode{};
    Command _currentCommand{};

    std::vector<IoSignal> _incomingSignals;
    std::vector<IoSignal> _outgoingSignals;
    std::vector<IoSignalContainer> _incomingSignalContainers;
    std::vector<IoSignalContainer> _outgoingSignalContainers;

    std::vector<CanController> _canControllers;
    std::vector<EthController> _ethControllers;
    std::vector<LinController> _linControllers;
    std::vector<CanControllerContainer> _canControllerContainers;
    std::vector<EthControllerContainer> _ethControllerContainers;
    std::vector<LinControllerContainer> _linControllerContainers;

    IoBuffer _ioBuffer;
    BusBuffer _busBuffer;
};

}  // namespace DsVeosCoSim
