// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <atomic>
#include <vector>

#include "BusBuffer.h"
#include "CoSimTypes.h"
#include "IoBuffer.h"

namespace DsVeosCoSim {

enum class ResponderMode {
    Unknown = 0,
    Blocking = 1,
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
    [[nodiscard]] DsVeosCoSim_ConnectionState GetConnectionState() const;

    [[nodiscard]] DsVeosCoSim_SimulationTime GetStepSize() const;

    [[nodiscard]] bool RunCallbackBasedCoSimulation(const Callbacks& callbacks);
    void StartPollingBasedCoSimulation(const Callbacks& callbacks);
    [[nodiscard]] bool PollCommand(DsVeosCoSim_SimulationTime& simulationTime, Command& command, bool returnOnPing);
    [[nodiscard]] bool FinishCommand();
    void SetNextSimulationTime(DsVeosCoSim_SimulationTime simulationTime);

    void Start();
    void Stop();
    void Terminate(DsVeosCoSim_TerminateReason terminateReason);
    void Pause();
    void Continue();

    void GetIncomingSignals(uint32_t* incomingSignalsCount, const DsVeosCoSim_IoSignal** incomingSignals) const;
    void GetOutgoingSignals(uint32_t* outgoingSignalsCount, const DsVeosCoSim_IoSignal** outgoingSignals) const;

    [[nodiscard]] std::vector<IoSignal> GetIncomingSignals() const;
    [[nodiscard]] std::vector<IoSignal> GetOutgoingSignals() const;

    void Write(DsVeosCoSim_IoSignalId outgoingSignalId, uint32_t length, const void* value) const;

    void Read(DsVeosCoSim_IoSignalId incomingSignalId, uint32_t& length, void* value) const;
    void Read(DsVeosCoSim_IoSignalId incomingSignalId, uint32_t& length, const void** value) const;

    void GetCanControllers(uint32_t* controllersCount, const DsVeosCoSim_CanController** controllers) const;
    void GetEthControllers(uint32_t* controllersCount, const DsVeosCoSim_EthController** controllers) const;
    void GetLinControllers(uint32_t* controllersCount, const DsVeosCoSim_LinController** controllers) const;

    [[nodiscard]] std::vector<CanController> GetCanControllers() const;
    [[nodiscard]] std::vector<EthController> GetEthControllers() const;
    [[nodiscard]] std::vector<LinController> GetLinControllers() const;

    [[nodiscard]] bool Transmit(const DsVeosCoSim_CanMessage& message) const;
    [[nodiscard]] bool Transmit(const DsVeosCoSim_EthMessage& message) const;
    [[nodiscard]] bool Transmit(const DsVeosCoSim_LinMessage& message) const;

    [[nodiscard]] bool Receive(DsVeosCoSim_CanMessage& message) const;
    [[nodiscard]] bool Receive(DsVeosCoSim_EthMessage& message) const;
    [[nodiscard]] bool Receive(DsVeosCoSim_LinMessage& message) const;

private:
    void SetCallbacks(const Callbacks& callbacks);
    void ResetDataFromPreviousConnect();

    [[nodiscard]] bool LocalConnect();
    [[nodiscard]] bool RemoteConnect();

    [[nodiscard]] bool SendConnectRequest() const;
    [[nodiscard]] bool OnConnectOk();
    [[nodiscard]] bool OnConnectError() const;
    [[nodiscard]] bool ReceiveConnectResponse();

    [[nodiscard]] bool RunCallbackBasedCoSimulationInternal();
    [[nodiscard]] bool PollCommandInternal(DsVeosCoSim_SimulationTime& simulationTime,
                                           Command& command,
                                           bool returnOnPing);
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
    DsVeosCoSim_SimulationTime _currentSimulationTime{};
    DsVeosCoSim_SimulationTime _nextSimulationTime{};

    DsVeosCoSim_SimulationTime _stepSize{};

    std::string _remoteIpAddress;
    std::string _serverName;
    std::string _clientName;
    uint16_t _remotePort{};
    uint16_t _localPort{};

    ResponderMode _responderMode{};
    Command _currentCommand{};
    std::atomic<Command> _nextCommand;

    std::vector<IoSignal> _incomingSignals;
    std::vector<IoSignal> _outgoingSignals;
    std::vector<DsVeosCoSim_IoSignal> _incomingSignalsExtern;
    std::vector<DsVeosCoSim_IoSignal> _outgoingSignalsExtern;

    std::vector<CanController> _canControllers;
    std::vector<EthController> _ethControllers;
    std::vector<LinController> _linControllers;
    std::vector<DsVeosCoSim_CanController> _canControllersExtern;
    std::vector<DsVeosCoSim_EthController> _ethControllersExtern;
    std::vector<DsVeosCoSim_LinController> _linControllersExtern;

    std::unique_ptr<IoBuffer> _ioBuffer;
    std::unique_ptr<BusBuffer> _busBuffer;
};

}  // namespace DsVeosCoSim
