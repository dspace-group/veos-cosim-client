// Copyright dSPACE GmbH. All rights reserved.

#include "DsVeosCoSim/DsVeosCoSim.h"

#include <memory>
#include <string>
#include <string_view>

#include "CoSimClient.h"
#include "CoSimHelper.h"
#include "CoSimTypes.h"

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
    SetLogCallback([](DsVeosCoSim_Severity severity, std::string_view message) {
        if (g_logCallback) {
            g_logCallback(severity, message.data());
        }
    });
}

DsVeosCoSim_Handle DsVeosCoSim_Create() {
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
        config.remoteIpAddress = connectConfig.remoteIpAddress;
    }

    if (connectConfig.serverName) {
        config.serverName = connectConfig.serverName;
    }

    if (connectConfig.clientName) {
        config.clientName = connectConfig.clientName;
    }

    config.remotePort = connectConfig.remotePort;
    config.localPort = connectConfig.localPort;

    try {
        if (client->Connect(config)) {
            return DsVeosCoSim_Result_Ok;
        }

        return DsVeosCoSim_Result_Disconnected;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_Disconnect(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->Disconnect();

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetConnectionState(DsVeosCoSim_Handle handle,
                                                  DsVeosCoSim_ConnectionState* connectionState) {
    CheckNotNull(handle);
    CheckNotNull(connectionState);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        *connectionState = client->GetConnectionState();

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_RunCallbackBasedCoSimulation(DsVeosCoSim_Handle handle,
                                                            DsVeosCoSim_Callbacks callbacks) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    Callbacks newCallbacks{};
    newCallbacks.callbacks = callbacks;

    try {
        if (client->RunCallbackBasedCoSimulation(newCallbacks)) {
            return DsVeosCoSim_Result_Ok;
        }

        return DsVeosCoSim_Result_Disconnected;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_StartPollingBasedCoSimulation(DsVeosCoSim_Handle handle,
                                                             DsVeosCoSim_Callbacks callbacks) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    Callbacks newCallbacks{};
    newCallbacks.callbacks = callbacks;

    try {
        client->StartPollingBasedCoSimulation(newCallbacks);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_PollCommand(DsVeosCoSim_Handle handle,
                                           DsVeosCoSim_SimulationTime* simulationTime,
                                           DsVeosCoSim_Command* command) {
    CheckNotNull(handle);
    CheckNotNull(simulationTime);
    CheckNotNull(command);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (client->PollCommand(*simulationTime, *reinterpret_cast<Command*>(command), false)) {
            return DsVeosCoSim_Result_Ok;
        }

        return DsVeosCoSim_Result_Disconnected;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_FinishCommand(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (client->FinishCommand()) {
            return DsVeosCoSim_Result_Ok;
        }

        return DsVeosCoSim_Result_Disconnected;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_SetNextSimulationTime(DsVeosCoSim_Handle handle,
                                                     DsVeosCoSim_SimulationTime simulationTime) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->SetNextSimulationTime(simulationTime);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetStepSize(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime* stepSize) {
    CheckNotNull(handle);
    CheckNotNull(stepSize);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        *stepSize = client->GetStepSize();

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetIncomingSignals(DsVeosCoSim_Handle handle,
                                                  uint32_t* incomingSignalsCount,
                                                  const DsVeosCoSim_IoSignal** incomingSignals) {
    CheckNotNull(handle);
    CheckNotNull(incomingSignalsCount);
    CheckNotNull(incomingSignals);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->GetIncomingSignals(incomingSignalsCount, incomingSignals);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_ReadIncomingSignal(DsVeosCoSim_Handle handle,
                                                  DsVeosCoSim_IoSignalId incomingSignalId,
                                                  uint32_t* length,
                                                  void* value) {
    CheckNotNull(handle);
    CheckNotNull(length);
    CheckNotNull(value);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->Read(incomingSignalId, *length, value);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetOutgoingSignals(DsVeosCoSim_Handle handle,
                                                  uint32_t* outgoingSignalsCount,
                                                  const DsVeosCoSim_IoSignal** outgoingSignals) {
    CheckNotNull(handle);
    CheckNotNull(outgoingSignalsCount);
    CheckNotNull(outgoingSignals);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->GetOutgoingSignals(outgoingSignalsCount, outgoingSignals);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_WriteOutgoingSignal(DsVeosCoSim_Handle handle,
                                                   DsVeosCoSim_IoSignalId outgoingSignalId,
                                                   uint32_t length,
                                                   const void* value) {
    CheckNotNull(handle);
    if (length > 0) {
        CheckNotNull(value);
    }

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->Write(outgoingSignalId, length, value);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetCanControllers(DsVeosCoSim_Handle handle,
                                                 uint32_t* canControllersCount,
                                                 const DsVeosCoSim_CanController** canControllers) {
    CheckNotNull(handle);
    CheckNotNull(canControllersCount);
    CheckNotNull(canControllers);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->GetCanControllers(canControllersCount, canControllers);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveCanMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (!client->Receive(*message)) {
            return DsVeosCoSim_Result_Empty;
        }

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_TransmitCanMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (!client->Transmit(*message)) {
            return DsVeosCoSim_Result_Full;
        }

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetEthControllers(DsVeosCoSim_Handle handle,
                                                 uint32_t* ethControllersCount,
                                                 const DsVeosCoSim_EthController** ethControllers) {
    CheckNotNull(handle);
    CheckNotNull(ethControllersCount);
    CheckNotNull(ethControllers);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->GetEthControllers(ethControllersCount, ethControllers);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveEthMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (!client->Receive(*message)) {
            return DsVeosCoSim_Result_Empty;
        }

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_TransmitEthMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (!client->Transmit(*message)) {
            return DsVeosCoSim_Result_Full;
        }

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetLinControllers(DsVeosCoSim_Handle handle,
                                                 uint32_t* linControllersCount,
                                                 const DsVeosCoSim_LinController** linControllers) {
    CheckNotNull(handle);
    CheckNotNull(linControllersCount);
    CheckNotNull(linControllers);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->GetLinControllers(linControllersCount, linControllers);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveLinMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (!client->Receive(*message)) {
            return DsVeosCoSim_Result_Empty;
        }

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_TransmitLinMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (!client->Transmit(*message)) {
            return DsVeosCoSim_Result_Full;
        }

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_StartSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->Start();

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_StopSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->Stop();

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_PauseSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->Pause();

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_ContinueSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->Continue();

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_TerminateSimulation(DsVeosCoSim_Handle handle,
                                                   DsVeosCoSim_TerminateReason terminateReason) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->Terminate(terminateReason);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

std::string DsVeosCoSim_SimulationTimeToString(DsVeosCoSim_SimulationTime simulationTime) {
    return SimulationTimeToString(simulationTime);
}

std::string DsVeosCoSim_ResultToString(DsVeosCoSim_Result result) {
    return ToString(result);
}

std::string DsVeosCoSim_CommandToString(DsVeosCoSim_Command command) {
    return ToString(static_cast<Command>(command));
}

std::string DsVeosCoSim_SeverityToString(DsVeosCoSim_Severity severity) {
    return ToString(severity);
}

std::string DsVeosCoSim_TerminateReasonToString(DsVeosCoSim_TerminateReason terminateReason) {
    return ToString(terminateReason);
}

std::string DsVeosCoSim_ConnectionStateToString(DsVeosCoSim_ConnectionState connectionState) {
    return ToString(connectionState);
}

std::string DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType dataType) {
    return ToString(dataType);
}

std::string DsVeosCoSim_SizeKindToString(DsVeosCoSim_SizeKind sizeKind) {
    return ToString(sizeKind);
}

std::string DsVeosCoSim_IoSignalToString(const DsVeosCoSim_IoSignal& ioSignal) {
    return ToString(ioSignal);
}

std::string DsVeosCoSim_CanControllerToString(const DsVeosCoSim_CanController& controller) {
    return ToString(controller);
}

std::string DsVeosCoSim_EthControllerToString(const DsVeosCoSim_EthController& controller) {
    return ToString(controller);
}

std::string DsVeosCoSim_LinControllerToString(const DsVeosCoSim_LinController& controller) {
    return ToString(controller);
}

std::string DsVeosCoSim_ValueToString(DsVeosCoSim_DataType dataType, uint32_t length, const void* value) {
    return ValueToString(dataType, length, value);
}

std::string DsVeosCoSim_DataToString(const uint8_t* data, size_t dataLength, char separator) {
    return DataToString(data, dataLength, separator);
}

std::string DsVeosCoSim_IoDataToString(DsVeosCoSim_SimulationTime simulationTime,
                                       const DsVeosCoSim_IoSignal& ioSignal,
                                       uint32_t length,
                                       const void* value) {
    return IoDataToString(simulationTime, ioSignal, length, value);
}

std::string DsVeosCoSim_CanMessageToString(DsVeosCoSim_SimulationTime simulationTime,
                                           const DsVeosCoSim_CanController& controller,
                                           const DsVeosCoSim_CanMessage& message) {
    return CanMessageToString(simulationTime, controller, message);
}

std::string DsVeosCoSim_EthMessageToString(DsVeosCoSim_SimulationTime simulationTime,
                                           const DsVeosCoSim_EthController& controller,
                                           const DsVeosCoSim_EthMessage& message) {
    return EthMessageToString(simulationTime, controller, message);
}

std::string DsVeosCoSim_LinMessageToString(DsVeosCoSim_SimulationTime simulationTime,
                                           const DsVeosCoSim_LinController& controller,
                                           const DsVeosCoSim_LinMessage& message) {
    return LinMessageToString(simulationTime, controller, message);
}

std::string DsVeosCoSim_LinControllerTypeToString(DsVeosCoSim_LinControllerType linControllerType) {
    return ToString(linControllerType);
}

std::string DsVeosCoSim_CanMessageFlagsToString(DsVeosCoSim_CanMessageFlags flags) {
    return CanMessageFlagsToString(flags);
}

std::string DsVeosCoSim_EthMessageFlagsToString(DsVeosCoSim_EthMessageFlags flags) {
    return EthMessageFlagsToString(flags);
}

std::string DsVeosCoSim_LinMessageFlagsToString(DsVeosCoSim_LinMessageFlags flags) {
    return LinMessageFlagsToString(flags);
}

size_t DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType dataType) {
    return GetDataTypeSize(dataType);
}
