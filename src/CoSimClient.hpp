// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "BusExchange.hpp"
#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "Protocol.hpp"
#include "Result.hpp"
#include "SignalExchange.hpp"

namespace DsVeosCoSim {

class CoSimClient final {
public:
    CoSimClient();
    ~CoSimClient() noexcept = default;

    CoSimClient(const CoSimClient&) = delete;
    CoSimClient& operator=(const CoSimClient&) = delete;

    CoSimClient(CoSimClient&&) = delete;
    CoSimClient& operator=(CoSimClient&&) = delete;

    [[nodiscard]] Result Connect(const ConnectConfig& connectConfig);
    void Disconnect();
    [[nodiscard]] ConnectionState GetConnectionState() const;

    [[nodiscard]] Result GetStepSize(SimulationTime& stepSize) const;
    [[nodiscard]] Result GetCurrentSimulationTime(SimulationTime& simulationTime) const;
    [[nodiscard]] Result GetSimulationState(SimulationState& simulationState) const;

    [[nodiscard]] Result RunCallbackBasedCoSimulation(const Callbacks& callbacks);
    [[nodiscard]] Result StartPollingBasedCoSimulation(const Callbacks& callbacks);
    [[nodiscard]] Result PollCommand(SimulationTime& simulationTime, Command& command, uint32_t timeoutInMilliseconds);
    [[nodiscard]] Result FinishCommand();
    [[nodiscard]] Result SetNextSimulationTime(SimulationTime simulationTime);
    [[nodiscard]] Result GetRoundTripTime(SimulationTime& roundTripTime) const;

    [[nodiscard]] Result Start();
    [[nodiscard]] Result Stop();
    [[nodiscard]] Result Terminate(TerminateReason terminateReason);
    [[nodiscard]] Result Pause();
    [[nodiscard]] Result Continue();

    [[nodiscard]] Result GetIncomingSignals(uint32_t& signalsCount, const IoSignal*& signals) const;
    [[nodiscard]] Result GetOutgoingSignals(uint32_t& signalsCount, const IoSignal*& signals) const;

    [[nodiscard]] Result GetIncomingSignals(std::vector<IoSignal>& signals) const;
    [[nodiscard]] Result GetOutgoingSignals(std::vector<IoSignal>& signals) const;

    [[nodiscard]] Result Write(IoSignalId outgoingSignalId, uint32_t length, const void* value) const;

    [[nodiscard]] Result Read(IoSignalId incomingSignalId, uint32_t& length, void* value) const;
    [[nodiscard]] Result Read(IoSignalId incomingSignalId, uint32_t& length, const void** value) const;

    [[nodiscard]] Result GetCanControllers(uint32_t& controllersCount, const CanController*& controllers) const;
    [[nodiscard]] Result GetEthControllers(uint32_t& controllersCount, const EthController*& controllers) const;
    [[nodiscard]] Result GetLinControllers(uint32_t& controllersCount, const LinController*& controllers) const;
    [[nodiscard]] Result GetFrControllers(uint32_t& controllersCount, const FrController*& controllers) const;

    [[nodiscard]] Result GetCanControllers(std::vector<CanController>& controllers) const;
    [[nodiscard]] Result GetEthControllers(std::vector<EthController>& controllers) const;
    [[nodiscard]] Result GetLinControllers(std::vector<LinController>& controllers) const;
    [[nodiscard]] Result GetFrControllers(std::vector<FrController>& controllers) const;

    [[nodiscard]] Result Transmit(const CanMessage& message) const;
    [[nodiscard]] Result Transmit(const EthMessage& message) const;
    [[nodiscard]] Result Transmit(const LinMessage& message) const;
    [[nodiscard]] Result Transmit(const FrMessage& message) const;

    [[nodiscard]] Result Transmit(const CanMessageContainer& messageContainer) const;
    [[nodiscard]] Result Transmit(const EthMessageContainer& messageContainer) const;
    [[nodiscard]] Result Transmit(const LinMessageContainer& messageContainer) const;
    [[nodiscard]] Result Transmit(const FrMessageContainer& messageContainer) const;

    [[nodiscard]] Result Receive(CanMessage& message) const;
    [[nodiscard]] Result Receive(EthMessage& message) const;
    [[nodiscard]] Result Receive(LinMessage& message) const;
    [[nodiscard]] Result Receive(FrMessage& message) const;

    [[nodiscard]] Result Receive(CanMessageContainer& messageContainer) const;
    [[nodiscard]] Result Receive(EthMessageContainer& messageContainer) const;
    [[nodiscard]] Result Receive(LinMessageContainer& messageContainer) const;
    [[nodiscard]] Result Receive(FrMessageContainer& messageContainer) const;

private:
    enum class ResponderMode : uint32_t {
        Unknown,
        Blocking,
        NonBlocking
    };

    void ResetDataFromPreviousConnect();
    [[nodiscard]] Result ConnectInternal();
    [[nodiscard]] Result LocalConnect();
    [[nodiscard]] Result RemoteConnect();
    [[nodiscard]] Result SendConnectRequest() const;
    [[nodiscard]] Result OnConnectOk();
    [[nodiscard]] Result OnConnectError() const;
    [[nodiscard]] Result ReceiveConnectResponse();
    [[nodiscard]] Result RunCallbackBasedCoSimulationInternal();
    [[nodiscard]] Result PollCommandInternal(SimulationTime& simulationTime, Command& command, uint32_t timeoutInMilliseconds);
    [[nodiscard]] Result FinishCommandInternal();
    [[nodiscard]] Result OnStep();
    [[nodiscard]] Result OnStart();
    [[nodiscard]] Result OnStop();
    [[nodiscard]] Result OnTerminate();
    [[nodiscard]] Result OnPause();
    [[nodiscard]] Result OnContinue();
    [[nodiscard]] Result OnPing();
    [[nodiscard]] Result FinishStep();
    [[nodiscard]] Result FinishPing();
    [[nodiscard]] Result FinishCurrentCommand() const;
    [[nodiscard]] Result EnsureIsConnected() const;
    [[nodiscard]] Result EnsureIsInResponderModeBlocking();
    [[nodiscard]] Result EnsureIsInResponderModeNonBlocking();
    void CloseConnection();
    [[nodiscard]] static Result OnUnexpectedFrame(FrameKind frameKind);
    [[nodiscard]] static Result CheckCanMessage(CanMessageFlags flags, uint32_t length);

    std::unique_ptr<Channel> _channel;
    ConnectionKind _connectionKind = ConnectionKind::Remote;

    std::unique_ptr<IProtocol> _protocol;

    bool _isConnected{};
    Callbacks _callbacks{};
    SimulationTime _currentSimulationTime{};
    SimulationTime _nextSimulationTime{};
    SimulationTime _roundTripTime{};

    SimulationTime _stepSize{};

    SimulationState _simulationState{};

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
    std::vector<FrControllerContainer> _frControllers;
    std::vector<CanController> _canControllersExtern;
    std::vector<EthController> _ethControllersExtern;
    std::vector<LinController> _linControllersExtern;
    std::vector<FrController> _frControllersExtern;

    std::unique_ptr<SignalExchange> _signalExchange;
    std::unique_ptr<BusExchange> _busExchange;

    SerializeFunction _serializeIoData;
    SerializeFunction _serializeBusMessages;
    DeserializeFunction _deserializeIoData;
    DeserializeFunction _deserializeBusMessages;
};

}  // namespace DsVeosCoSim
