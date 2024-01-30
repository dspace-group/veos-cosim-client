// Copyright dSPACE GmbH. All rights reserved.

#include <memory>

#include "CoSimClient.h"
#include "CoSimTypes.h"
#include "Logger.h"

using namespace DsVeosCoSim;

namespace {

#define CheckNotNull(arg)                                    \
    do {                                                     \
        if (!(arg)) {                                        \
            LogError("Argument " #arg " must not be null."); \
            return DsVeosCoSim_Result_InvalidArgument;       \
        }                                                    \
    } while (0)

DsVeosCoSim_LogCallback g_logCallback;

}  // namespace

void DsVeosCoSim_SetLogCallback(DsVeosCoSim_LogCallback logCallback) {
    g_logCallback = logCallback;
    SetLogCallback([](Severity severity, std::string_view message) {
        if (g_logCallback) {
            g_logCallback(static_cast<DsVeosCoSim_Severity>(severity), message.data());
        }
    });
}

DsVeosCoSim_Handle DsVeosCoSim_Create() {
    if ((StartupNetwork()) != Result::Ok) {
        return nullptr;
    }

    auto client = std::make_unique<CoSimClient>();
    return client.release();
}

void DsVeosCoSim_Destroy(DsVeosCoSim_Handle handle) {
    if (!handle) {
        return;
    }

    const auto* const client = static_cast<CoSimClient*>(handle);

    delete client;
}

DsVeosCoSim_Result DsVeosCoSim_Connect(DsVeosCoSim_Handle handle, DsVeosCoSim_ConnectConfig connectConfig) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    ConnectConfig config{};
    if (connectConfig.remoteIpAddress) {
        config.remoteIpAddress = std::string(connectConfig.remoteIpAddress);
    }

    if (connectConfig.serverName) {
        config.serverName = std::string(connectConfig.serverName);
    }

    if (connectConfig.clientName) {
        config.clientName = std::string(connectConfig.clientName);
    }

    config.remotePort = connectConfig.remotePort;
    config.localPort = connectConfig.localPort;

    return static_cast<DsVeosCoSim_Result>(client->Connect(config));
}

DsVeosCoSim_Result DsVeosCoSim_Disconnect(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    client->Disconnect();

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_GetConnectionState(DsVeosCoSim_Handle handle, DsVeosCoSim_ConnectionState* connectionState) {
    CheckNotNull(handle);
    CheckNotNull(connectionState);

    const auto* const client = static_cast<CoSimClient*>(handle);

    *connectionState = static_cast<DsVeosCoSim_ConnectionState>(client->GetConnectionState());

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_RunCallbackBasedCoSimulation(DsVeosCoSim_Handle handle, DsVeosCoSim_Callbacks callbacks) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    Callbacks newCallbacks{};
    newCallbacks.callbacks = callbacks;

    return static_cast<DsVeosCoSim_Result>(client->RunCallbackBasedCoSimulation(newCallbacks));
}

DsVeosCoSim_Result DsVeosCoSim_StartPollingBasedCoSimulation(DsVeosCoSim_Handle handle, DsVeosCoSim_Callbacks callbacks) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    Callbacks newCallbacks{};
    newCallbacks.callbacks = callbacks;

    return static_cast<DsVeosCoSim_Result>(client->StartPollingBasedCoSimulation(newCallbacks));
}

DsVeosCoSim_Result DsVeosCoSim_PollCommand(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime* simulationTime, DsVeosCoSim_Command* command) {
    CheckNotNull(handle);
    CheckNotNull(simulationTime);
    CheckNotNull(command);

    auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->PollCommand(*simulationTime, *reinterpret_cast<Command*>(command), false));
}

DsVeosCoSim_Result DsVeosCoSim_FinishCommand(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->FinishCommand());
}

DsVeosCoSim_Result DsVeosCoSim_SetNextSimulationTime(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime simulationTime) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->SetNextSimulationTime(simulationTime));
}

DsVeosCoSim_Result DsVeosCoSim_GetStepSize(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime* stepSize) {
    CheckNotNull(handle);
    CheckNotNull(stepSize);

    const auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetStepSize(*stepSize));
}

DsVeosCoSim_Result DsVeosCoSim_GetIncomingSignals(DsVeosCoSim_Handle handle, uint32_t* incomingSignalsCount, const DsVeosCoSim_IoSignal** incomingSignals) {
    CheckNotNull(handle);
    CheckNotNull(incomingSignalsCount);
    CheckNotNull(incomingSignals);

    const auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetIncomingSignals(incomingSignalsCount, incomingSignals));
}

DsVeosCoSim_Result DsVeosCoSim_ReadIncomingSignal(DsVeosCoSim_Handle handle, DsVeosCoSim_IoSignalId incomingSignalId, uint32_t* length, void* value) {
    CheckNotNull(handle);
    CheckNotNull(length);
    CheckNotNull(value);

    auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Read(static_cast<IoSignalId>(incomingSignalId), *length, value));
}

DsVeosCoSim_Result DsVeosCoSim_GetOutgoingSignals(DsVeosCoSim_Handle handle, uint32_t* outgoingSignalsCount, const DsVeosCoSim_IoSignal** outgoingSignals) {
    CheckNotNull(handle);
    CheckNotNull(outgoingSignalsCount);
    CheckNotNull(outgoingSignals);

    const auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetOutgoingSignals(outgoingSignalsCount, outgoingSignals));
}

DsVeosCoSim_Result DsVeosCoSim_WriteOutgoingSignal(DsVeosCoSim_Handle handle, DsVeosCoSim_IoSignalId outgoingSignalId, uint32_t length, const void* value) {
    CheckNotNull(handle);
    if (length > 0) {
        CheckNotNull(value);
    }

    auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Write(static_cast<IoSignalId>(outgoingSignalId), length, value));
}

DsVeosCoSim_Result DsVeosCoSim_GetCanControllers(DsVeosCoSim_Handle handle, uint32_t* canControllersCount, const DsVeosCoSim_CanController** canControllers) {
    CheckNotNull(handle);
    CheckNotNull(canControllersCount);
    CheckNotNull(canControllers);

    const auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetControllers(canControllersCount, canControllers));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveCanMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(*reinterpret_cast<CanMessage*>(message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitCanMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(*reinterpret_cast<const CanMessage*>(message)));
}

DsVeosCoSim_Result DsVeosCoSim_GetEthControllers(DsVeosCoSim_Handle handle, uint32_t* ethControllersCount, const DsVeosCoSim_EthController** ethControllers) {
    CheckNotNull(handle);
    CheckNotNull(ethControllersCount);
    CheckNotNull(ethControllers);

    const auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetControllers(ethControllersCount, ethControllers));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveEthMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(*reinterpret_cast<EthMessage*>(message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitEthMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(*reinterpret_cast<const EthMessage*>(message)));
}

DsVeosCoSim_Result DsVeosCoSim_GetLinControllers(DsVeosCoSim_Handle handle, uint32_t* linControllersCount, const DsVeosCoSim_LinController** linControllers) {
    CheckNotNull(handle);
    CheckNotNull(linControllersCount);
    CheckNotNull(linControllers);

    const auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetControllers(linControllersCount, linControllers));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveLinMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(*reinterpret_cast<LinMessage*>(message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitLinMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* const client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(*reinterpret_cast<const LinMessage*>(message)));
}
