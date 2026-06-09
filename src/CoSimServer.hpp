// Copyright dSPACE SE & Co. KG. All rights reserved.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "BusExchange.hpp"
#include "Channel.hpp"
#include "CoSimTypes.hpp"
#include "PortMapper.hpp"
#include "Protocol.hpp"
#include "Result.hpp"
#include "SignalExchange.hpp"

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
    IncomingSignalChangedCallback incomingSignalChangedCallback;
    CanMessageContainerReceivedCallback canMessageContainerReceivedCallback;
    LinMessageContainerReceivedCallback linMessageContainerReceivedCallback;
    FrMessageContainerReceivedCallback frMessageContainerReceivedCallback;
    EthMessageContainerReceivedCallback ethMessageContainerReceivedCallback;
    std::vector<IoSignalContainer> incomingSignals;
    std::vector<IoSignalContainer> outgoingSignals;
    std::vector<CanControllerContainer> canControllers;
    std::vector<EthControllerContainer> ethControllers;
    std::vector<LinControllerContainer> linControllers;
    std::vector<FrControllerContainer> frControllers;
};

class CoSimServer final {
public:
    CoSimServer();
    ~CoSimServer() noexcept;

    CoSimServer(const CoSimServer&) = delete;
    CoSimServer& operator=(const CoSimServer&) = delete;

    CoSimServer(CoSimServer&&) = delete;
    CoSimServer& operator=(CoSimServer&&) = delete;

    [[nodiscard]] Result Load(const CoSimServerConfig& config);
    void Unload();

    [[nodiscard]] Result Start(SimulationTime simulationTime);
    [[nodiscard]] Result Stop(SimulationTime simulationTime);
    [[nodiscard]] Result Terminate(SimulationTime simulationTime, TerminateReason reason);
    [[nodiscard]] Result Pause(SimulationTime simulationTime);
    [[nodiscard]] Result Continue(SimulationTime simulationTime);
    [[nodiscard]] Result Step(SimulationTime simulationTime, SimulationTime& nextSimulationTime);

    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value) const;

    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value, bool& valueRead) const;

    [[nodiscard]] Result Transmit(const CanMessage& message) const;
    [[nodiscard]] Result Transmit(const EthMessage& message) const;
    [[nodiscard]] Result Transmit(const LinMessage& message) const;
    [[nodiscard]] Result Transmit(const FrMessage& message) const;

    [[nodiscard]] Result Transmit(const CanMessageContainer& messageContainer) const;
    [[nodiscard]] Result Transmit(const EthMessageContainer& messageContainer) const;
    [[nodiscard]] Result Transmit(const LinMessageContainer& messageContainer) const;
    [[nodiscard]] Result Transmit(const FrMessageContainer& messageContainer) const;

    [[nodiscard]] Result BackgroundService(SimulationTime& roundTripTime);

    [[nodiscard]] Result GetLocalPort(uint16_t& localPort) const;

private:
    [[nodiscard]] Result StartInternal(SimulationTime simulationTime) const;
    [[nodiscard]] Result StopInternal(SimulationTime simulationTime) const;
    [[nodiscard]] Result TerminateInternal(SimulationTime simulationTime, TerminateReason reason) const;
    [[nodiscard]] Result PauseInternal(SimulationTime simulationTime) const;
    [[nodiscard]] Result ContinueInternal(SimulationTime simulationTime) const;
    [[nodiscard]] Result StepInternal(SimulationTime simulationTime, SimulationTime& nextSimulationTime, Command& command);
    [[nodiscard]] Result CloseConnection();
    [[nodiscard]] Result Ping(Command& command);
    [[nodiscard]] Result StartAccepting();
    void StopAccepting();
    [[nodiscard]] Result AcceptChannel();
    [[nodiscard]] Result OnHandleConnect();
    [[nodiscard]] Result WaitForOkFrame() const;
    [[nodiscard]] Result WaitForPingOkFrame(Command& command) const;
    [[nodiscard]] Result WaitForConnectFrame(uint32_t& version, std::string& clientName) const;
    [[nodiscard]] Result WaitForStepOkFrame(SimulationTime& simulationTime, Command& command) const;
    [[nodiscard]] Result OnError() const;
    void HandlePendingCommand(Command command) const;
    [[nodiscard]] static Result OnUnexpectedFrame(FrameKind frameKind);

    std::unique_ptr<Channel> _channel;
    std::unique_ptr<IProtocol> _protocol;
    uint16_t _localPort{};
    bool _enableRemoteAccess{};
    std::unique_ptr<PortMapperServer> _portMapperServer;
    std::unique_ptr<ChannelServer> _tcpChannelServer;
    std::unique_ptr<ChannelServer> _localChannelServer;
    ConnectionKind _connectionKind = ConnectionKind::Remote;
    std::string _serverName;
    Callbacks _callbacks{};
    bool _isClientOptional{};
    SimulationTime _stepSize{};
    SimulationState _simulationState{};
    bool _registerAtPortMapper{};
    SimulationTime _roundTripTime{};
    bool _firstStep{true};
    std::vector<IoSignalContainer> _incomingSignals;
    std::vector<IoSignalContainer> _outgoingSignals;
    std::vector<CanControllerContainer> _canControllers;
    std::vector<EthControllerContainer> _ethControllers;
    std::vector<LinControllerContainer> _linControllers;
    std::vector<FrControllerContainer> _frControllers;
    std::unique_ptr<SignalExchange> _signalExchange;
    std::unique_ptr<BusExchange> _busExchange;
    SerializeFunction _serializeIoData;
    SerializeFunction _serializeBusMessages;
    DeserializeFunction _deserializeIoData;
    DeserializeFunction _deserializeBusMessages;
};

}  // namespace DsVeosCoSim
