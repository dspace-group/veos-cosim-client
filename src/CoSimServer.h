// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>

#include "BusBuffer.h"
#include "CoSimTypes.h"
#include "IoBuffer.h"
#include "PortMapper.h"
#include "SocketChannel.h"

#ifdef _WIN32
#include "LocalChannel.h"
#endif

namespace DsVeosCoSim {

struct CoSimServerConfig {
    uint16_t port{};
    bool enableRemoteAccess{};
    std::string serverName;
    bool isClientOptional{};
    bool startPortMapper{};
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
    std::vector<IoSignalContainer> incomingSignals;
    std::vector<IoSignalContainer> outgoingSignals;
    std::vector<CanControllerContainer> canControllers;
    std::vector<EthControllerContainer> ethControllers;
    std::vector<LinControllerContainer> linControllers;
};

class CoSimServer final {
public:
    CoSimServer() = default;
    ~CoSimServer() noexcept;

    CoSimServer(const CoSimServer&) = delete;
    CoSimServer& operator=(const CoSimServer&) = delete;

    CoSimServer(CoSimServer&&) = delete;
    CoSimServer& operator=(CoSimServer&&) = delete;

    void Load(const CoSimServerConfig& config);
    void Unload();

    void Start(SimulationTime simulationTime);
    void Stop(SimulationTime simulationTime);
    void Terminate(SimulationTime simulationTime, TerminateReason reason);
    void Pause(SimulationTime simulationTime);
    void Continue(SimulationTime simulationTime);
    void Step(SimulationTime simulationTime, SimulationTime& nextSimulationTime);

    void Write(IoSignalId signalId, uint32_t length, const void* value) const;

    void Read(IoSignalId signalId, uint32_t& length, const void** value) const;

    [[nodiscard]] bool Transmit(const CanMessage& message) const;
    [[nodiscard]] bool Transmit(const EthMessage& message) const;
    [[nodiscard]] bool Transmit(const LinMessage& message) const;

    void BackgroundService();

    [[nodiscard]] uint16_t GetLocalPort() const;

private:
    [[nodiscard]] bool StartInternal(SimulationTime simulationTime) const;
    [[nodiscard]] bool StopInternal(SimulationTime simulationTime) const;
    [[nodiscard]] bool TerminateInternal(SimulationTime simulationTime, TerminateReason reason) const;
    [[nodiscard]] bool PauseInternal(SimulationTime simulationTime) const;
    [[nodiscard]] bool ContinueInternal(SimulationTime simulationTime) const;
    [[nodiscard]] bool StepInternal(SimulationTime simulationTime,
                                    SimulationTime& nextSimulationTime,
                                    Command& command) const;

    void CloseConnection();
    [[nodiscard]] bool Ping(Command& command) const;
    void StartAccepting();
    void StopAccepting();
    [[nodiscard]] bool AcceptChannel();
    [[nodiscard]] bool OnHandleConnect();

    [[nodiscard]] bool WaitForOkFrame() const;
    [[nodiscard]] bool WaitForPingOkFrame(Command& command) const;
    [[nodiscard]] bool WaitForConnectFrame(uint32_t& version, std::string& clientName) const;
    [[nodiscard]] bool WaitForStepOkFrame(SimulationTime& simulationTime, Command& command) const;

    void HandlePendingCommand(Command command) const;

    std::unique_ptr<Channel> _channel;

    uint16_t _localPort{};
    bool _enableRemoteAccess{};

    std::unique_ptr<PortMapperServer> _portMapperServer;
    std::unique_ptr<TcpChannelServer> _tcpChannelServer;
#ifdef _WIN32
    std::unique_ptr<LocalChannelServer> _localChannelServer;
#else
    std::unique_ptr<UdsChannelServer> _udsChannelServer;
#endif

    ConnectionKind _connectionKind = ConnectionKind::Remote;
    std::string _serverName;
    Callbacks _callbacks{};
    bool _isClientOptional{};
    SimulationTime _stepSize{};
    bool _registerAtPortMapper{};

    std::vector<IoSignalContainer> _incomingSignals;
    std::vector<IoSignalContainer> _outgoingSignals;
    std::vector<CanControllerContainer> _canControllers;
    std::vector<EthControllerContainer> _ethControllers;
    std::vector<LinControllerContainer> _linControllers;
    std::unique_ptr<IoBuffer> _ioBuffer;
    std::unique_ptr<BusBuffer> _busBuffer;
};

}  // namespace DsVeosCoSim
