// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <string>
#include <thread>
#include <vector>

#include "BusBuffer.h"
#include "CoSimTypes.h"
#include "Communication.h"
#include "IoBuffer.h"
#include "PortMapper.h"

namespace DsVeosCoSim {

struct CoSimServerConfig {
    std::string serverName;
    uint16_t localPort = 0;
    bool isClientOptional = false;
    bool enableRemoteAccess = true;
    bool startPortMapper = false;
    bool registerAtPortMapper = true;
    SimulationTime stepSize{};
    LogCallback logCallback;
    SimulationCallback simulationStartedCallback;
    SimulationCallback simulationStoppedCallback;
    SimulationTerminatedCallback simulationTerminatedCallback;
    SimulationCallback simulationPausedCallback;
    SimulationCallback simulationContinuedCallback;
    CanMessageReceivedCallback canMessageReceivedCallback;
    LinMessageReceivedCallback linMessageReceivedCallback;
    EthMessageReceivedCallback ethMessageReceivedCallback;
    std::vector<IoSignal> incomingSignals;
    std::vector<IoSignal> outgoingSignals;
    std::vector<CanController> canControllers;
    std::vector<EthController> ethControllers;
    std::vector<LinController> linControllers;
};

class CoSimServer final {
public:
    CoSimServer() = default;
    ~CoSimServer() noexcept;

    CoSimServer(const CoSimServer&) = delete;
    CoSimServer& operator=(const CoSimServer&) = delete;

    CoSimServer(CoSimServer&&) = delete;
    CoSimServer& operator=(CoSimServer&&) = delete;

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
    [[nodiscard]] Result Read(IoSignalId signalId, uint32_t& length, const void** value);
    [[nodiscard]] Result Write(IoSignalId signalId, uint32_t length, const void* value);

    [[nodiscard]] Result Transmit(const DsVeosCoSim_CanMessage& message);
    [[nodiscard]] Result Transmit(const DsVeosCoSim_EthMessage& message);
    [[nodiscard]] Result Transmit(const DsVeosCoSim_LinMessage& message);

    [[nodiscard]] Result BackgroundService();

    [[nodiscard]] uint16_t GetLocalPort() const;

private:
    [[nodiscard]] Result StartInternal(SimulationTime simulationTime);
    [[nodiscard]] Result StopInternal(SimulationTime simulationTime);
    [[nodiscard]] Result TerminateInternal(SimulationTime simulationTime, TerminateReason reason);
    [[nodiscard]] Result PauseInternal(SimulationTime simulationTime);
    [[nodiscard]] Result ContinueInternal(SimulationTime simulationTime);
    [[nodiscard]] Result StepInternal(SimulationTime simulationTime, SimulationTime& nextSimulationTime, Command& command);

    [[nodiscard]] Result CloseConnection();
    [[nodiscard]] Result Ping(Command& command);
    [[nodiscard]] Result OnHandleConnect(std::string_view remoteIpAddress, uint16_t remotePort);
    [[nodiscard]] Result StartAccepting();
    [[nodiscard]] Result Accepting();

    [[nodiscard]] Result WaitForOkFrame();
    [[nodiscard]] Result WaitForPingOkFrame(Command& command);
    [[nodiscard]] Result WaitForConnectFrame(uint32_t& version, std::string& clientName);
    [[nodiscard]] Result WaitForStepOkFrame(SimulationTime& simulationTime, Command& command);

    void HandlePendingCommand(Command command) const;

    Channel _channel;

    PortMapperServer _portMapperServer;
    Server _server;

    bool _isConnected{};
    uint16_t _actualLocalPort{};
    uint16_t _givenLocalPort{};
    std::string _serverName;
    Callbacks _callbacks{};
    bool _isClientOptional{};
    bool _enableRemoteAccess{};
    SimulationTime _stepSize{};
    bool _registerAtPortMapper{};

    std::vector<IoSignal> _incomingSignals;
    std::vector<IoSignal> _outgoingSignals;
    std::vector<CanController> _canControllers;
    std::vector<EthController> _ethControllers;
    std::vector<LinController> _linControllers;
    IoBuffer _ioBuffer;
    BusBuffer _busBuffer;

    bool _stopAcceptingThread{};
    std::thread _acceptingThread;
};

}  // namespace DsVeosCoSim
