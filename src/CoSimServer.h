// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <functional>
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
    LogCallback logCallback{};
    SimulationCallback simulationStoppedCallback;
    CanMessageReceivedCallback canMessageReceivedCallback;
    LinMessageReceivedCallback linMessageReceivedCallback;
    EthMessageReceivedCallback ethMessageReceivedCallback;
    std::vector<IoSignalContainer> incomingSignals;
    std::vector<IoSignalContainer> outgoingSignals;
    std::vector<CanControllerContainer> canControllers;
    std::vector<EthControllerContainer> ethControllers;
    std::vector<LinControllerContainer> linControllers;
};

class CoSimServer final {
public:
    CoSimServer() = default;
    ~CoSimServer();

    CoSimServer(const CoSimServer&) = delete;
    CoSimServer& operator=(CoSimServer const&) = delete;

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

    [[nodiscard]] Result Transmit(const CanMessage& message);
    [[nodiscard]] Result Transmit(const LinMessage& message);
    [[nodiscard]] Result Transmit(const EthMessage& message);

    [[nodiscard]] Result BackgroundService();

private:
    [[nodiscard]] Result StartInternal(SimulationTime simulationTime);
    [[nodiscard]] Result StopInternal(SimulationTime simulationTime);
    [[nodiscard]] Result TerminateInternal(SimulationTime simulationTime, TerminateReason reason);
    [[nodiscard]] Result PauseInternal(SimulationTime simulationTime);
    [[nodiscard]] Result ContinueInternal(SimulationTime simulationTime);
    [[nodiscard]] Result StepInternal(SimulationTime simulationTime, SimulationTime& nextSimulationTime);

    [[nodiscard]] Result UpdateTime();
    [[nodiscard]] Result CloseConnection();
    [[nodiscard]] Result Ping();
    [[nodiscard]] Result OnHandleConnect(std::string_view remoteIpAddress, uint16_t remotePort);
    [[nodiscard]] Result StartAccepting();
    [[nodiscard]] Result Accepting();

    [[nodiscard]] Result WaitForOkFrame();
    [[nodiscard]] Result WaitForConnectFrame(uint32_t& version, std::string& clientName);
    [[nodiscard]] Result WaitForStepResponseFrame(SimulationTime& simulationTime);

    Channel _channel;

    PortMapperServer _portMapperServer;
    Server _server;

    bool _isConnected{};
    uint16_t _localPort{};
    std::string _serverName;
    Callbacks _callbacks{};
    bool _isClientOptional{};
    bool _enableRemoteAccess{};
    bool _isPortKnownToPortMapper{};

    std::time_t _lastCommandSentOrReceived{};

    std::vector<IoSignalContainer> _incomingSignals;
    std::vector<IoSignalContainer> _outgoingSignals;
    std::vector<CanControllerContainer> _canControllers;
    std::vector<EthControllerContainer> _ethControllers;
    std::vector<LinControllerContainer> _linControllers;
    IoBuffer _ioBuffer;
    BusBuffer _busBuffer;

    bool _stopAcceptingThread{};
    std::thread _acceptingThread;
};

}  // namespace DsVeosCoSim
