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
    DsVeosCoSim_SimulationTime stepSize{};
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

    void Load(const CoSimServerConfig& config);
    void Unload();

    void Start(DsVeosCoSim_SimulationTime simulationTime);
    void Stop(DsVeosCoSim_SimulationTime simulationTime);
    void Terminate(DsVeosCoSim_SimulationTime simulationTime, DsVeosCoSim_TerminateReason reason);
    void Pause(DsVeosCoSim_SimulationTime simulationTime);
    void Continue(DsVeosCoSim_SimulationTime simulationTime);
    void Step(DsVeosCoSim_SimulationTime simulationTime, DsVeosCoSim_SimulationTime& nextSimulationTime);

    void Write(DsVeosCoSim_IoSignalId signalId, uint32_t length, const void* value) const;

    void Read(DsVeosCoSim_IoSignalId signalId, uint32_t& length, const void** value) const;

    [[nodiscard]] bool Transmit(const DsVeosCoSim_CanMessage& message) const;
    [[nodiscard]] bool Transmit(const DsVeosCoSim_EthMessage& message) const;
    [[nodiscard]] bool Transmit(const DsVeosCoSim_LinMessage& message) const;

    void BackgroundService();

    [[nodiscard]] uint16_t GetLocalPort() const;

private:
    [[nodiscard]] bool StartInternal(DsVeosCoSim_SimulationTime simulationTime) const;
    [[nodiscard]] bool StopInternal(DsVeosCoSim_SimulationTime simulationTime) const;
    [[nodiscard]] bool TerminateInternal(DsVeosCoSim_SimulationTime simulationTime,
                                         DsVeosCoSim_TerminateReason reason) const;
    [[nodiscard]] bool PauseInternal(DsVeosCoSim_SimulationTime simulationTime) const;
    [[nodiscard]] bool ContinueInternal(DsVeosCoSim_SimulationTime simulationTime) const;
    [[nodiscard]] bool StepInternal(DsVeosCoSim_SimulationTime simulationTime,
                                    DsVeosCoSim_SimulationTime& nextSimulationTime,
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
    [[nodiscard]] bool WaitForStepOkFrame(DsVeosCoSim_SimulationTime& simulationTime, Command& command) const;

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
    DsVeosCoSim_SimulationTime _stepSize{};
    bool _registerAtPortMapper{};

    std::vector<IoSignal> _incomingSignals;
    std::vector<IoSignal> _outgoingSignals;
    std::vector<CanController> _canControllers;
    std::vector<EthController> _ethControllers;
    std::vector<LinController> _linControllers;
    std::unique_ptr<IoBuffer> _ioBuffer;
    std::unique_ptr<BusBuffer> _busBuffer;
};

}  // namespace DsVeosCoSim
