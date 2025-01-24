// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <atomic>
#include <string>
#include <vector>

#include "BusBuffer.h"
#include "CoSimTypes.h"
#include "IoBuffer.h"

namespace DsVeosCoSim {

enum class ResponderMode {
    Unknown,
    Blocking,
    NonBlocking
};

class CoSimClient final {
public:
    CoSimClient() = default;
    ~CoSimClient() noexcept = default;

    CoSimClient(const CoSimClient&) = delete;
    CoSimClient& operator=(const CoSimClient&) = delete;

    CoSimClient(CoSimClient&&) = delete;
    CoSimClient& operator=(CoSimClient&&) = delete;

    [[nodiscard]] bool Connect(const ConnectConfig& connectConfig);
    void Disconnect();
    [[nodiscard]] ConnectionState GetConnectionState() const;

    [[nodiscard]] SimulationTime GetStepSize() const;

    [[nodiscard]] bool RunCallbackBasedCoSimulation(const Callbacks& callbacks);
    void StartPollingBasedCoSimulation(const Callbacks& callbacks);
    [[nodiscard]] bool PollCommand(SimulationTime& simulationTime, Command& command, bool returnOnPing);
    [[nodiscard]] bool FinishCommand();
    void SetNextSimulationTime(SimulationTime simulationTime);

    void Start();
    void Stop();
    void Terminate(TerminateReason terminateReason);
    void Pause();
    void Continue();

    void GetIncomingSignals(uint32_t* incomingSignalsCount, const IoSignal** incomingSignals) const;
    void GetOutgoingSignals(uint32_t* outgoingSignalsCount, const IoSignal** outgoingSignals) const;

    [[nodiscard]] std::vector<IoSignal> GetIncomingSignals() const;
    [[nodiscard]] std::vector<IoSignal> GetOutgoingSignals() const;

    void Write(IoSignalId outgoingSignalId, uint32_t length, const void* value) const;

    void Read(IoSignalId incomingSignalId, uint32_t& length, void* value) const;
    void Read(IoSignalId incomingSignalId, uint32_t& length, const void** value) const;

    void GetCanControllers(uint32_t* controllersCount, const CanController** controllers) const;
    void GetEthControllers(uint32_t* controllersCount, const EthController** controllers) const;
    void GetLinControllers(uint32_t* controllersCount, const LinController** controllers) const;

    [[nodiscard]] std::vector<CanController> GetCanControllers() const;
    [[nodiscard]] std::vector<EthController> GetEthControllers() const;
    [[nodiscard]] std::vector<LinController> GetLinControllers() const;

    [[nodiscard]] bool Transmit(const CanMessage& message) const;
    [[nodiscard]] bool Transmit(const EthMessage& message) const;
    [[nodiscard]] bool Transmit(const LinMessage& message) const;

    [[nodiscard]] bool Receive(CanMessage& message) const;
    [[nodiscard]] bool Receive(EthMessage& message) const;
    [[nodiscard]] bool Receive(LinMessage& message) const;

private:
    void ResetDataFromPreviousConnect();

    [[nodiscard]] bool LocalConnect();
    [[nodiscard]] bool RemoteConnect();

    [[nodiscard]] bool SendConnectRequest() const;
    [[nodiscard]] bool OnConnectOk();
    [[nodiscard]] bool OnConnectError() const;
    [[nodiscard]] bool ReceiveConnectResponse();

    [[nodiscard]] bool RunCallbackBasedCoSimulationInternal();
    [[nodiscard]] bool PollCommandInternal(SimulationTime& simulationTime, Command& command, bool returnOnPing);
    [[nodiscard]] bool FinishCommandInternal();

    [[nodiscard]] bool OnStep();
    [[nodiscard]] bool OnStart();
    [[nodiscard]] bool OnStop();
    [[nodiscard]] bool OnTerminate();
    [[nodiscard]] bool OnPause();
    [[nodiscard]] bool OnContinue();

    void EnsureIsConnected() const;
    void EnsureIsInResponderModeBlocking();
    void EnsureIsInResponderModeNonBlocking();

    void CloseConnection();

    std::unique_ptr<Channel> _channel;
    ConnectionKind _connectionKind = ConnectionKind::Remote;

    bool _isConnected{};
    Callbacks _callbacks{};
    SimulationTime _currentSimulationTime{};
    SimulationTime _nextSimulationTime{};

    SimulationTime _stepSize{};

    std::string _remoteIpAddress;
    std::string _serverName;
    std::string _clientName;
    uint16_t _remotePort{};
    uint16_t _localPort{};

    ResponderMode _responderMode{};
    Command _currentCommand{};
    std::atomic<Command> _nextCommand{};

    std::vector<IoSignalContainer> _incomingSignals;
    std::vector<IoSignalContainer> _outgoingSignals;
    std::vector<IoSignal> _incomingSignalsExtern;
    std::vector<IoSignal> _outgoingSignalsExtern;

    std::vector<CanControllerContainer> _canControllers;
    std::vector<EthControllerContainer> _ethControllers;
    std::vector<LinControllerContainer> _linControllers;
    std::vector<CanController> _canControllersExtern;
    std::vector<EthController> _ethControllersExtern;
    std::vector<LinController> _linControllersExtern;

    std::unique_ptr<IoBuffer> _ioBuffer;
    std::unique_ptr<BusBuffer> _busBuffer;
};

}  // namespace DsVeosCoSim
