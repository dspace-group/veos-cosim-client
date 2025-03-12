// Copyright dSPACE GmbH. All rights reserved.

#include "DsVeosCoSim/DsVeosCoSim.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <string_view>

#include "CoSimHelper.h"
#include "DsVeosCoSim/CoSimClient.h"
#include "DsVeosCoSim/CoSimTypes.h"

using namespace DsVeosCoSim;

namespace {

#define CheckNotNull(arg)                                    \
    do {                                                     \
        if (!(arg)) {                                        \
            LogError("Argument " #arg " must not be null."); \
            return DsVeosCoSim_Result_InvalidArgument;       \
        }                                                    \
    } while (0)

DsVeosCoSim_LogCallback LogCallbackHandler;

void InitializeCallbacks(Callbacks& newCallbacks, const DsVeosCoSim_Callbacks& callbacks) {
    const DsVeosCoSim_CanMessageReceivedCallback canMessageReceivedCallback = callbacks.canMessageReceivedCallback;
    const DsVeosCoSim_EthMessageReceivedCallback ethMessageReceivedCallback = callbacks.ethMessageReceivedCallback;
    const DsVeosCoSim_LinMessageReceivedCallback linMessageReceivedCallback = callbacks.linMessageReceivedCallback;
    const DsVeosCoSim_IncomingSignalChangedCallback incomingSignalChangedCallback =
        callbacks.incomingSignalChangedCallback;
    const DsVeosCoSim_SimulationCallback simulationStartedCallback = callbacks.simulationStartedCallback;
    const DsVeosCoSim_SimulationCallback simulationStoppedCallback = callbacks.simulationStoppedCallback;
    const DsVeosCoSim_SimulationCallback simulationPausedCallback = callbacks.simulationPausedCallback;
    const DsVeosCoSim_SimulationCallback simulationContinuedCallback = callbacks.simulationContinuedCallback;
    const DsVeosCoSim_SimulationTerminatedCallback simulationTerminatedCallback =
        callbacks.simulationTerminatedCallback;
    const DsVeosCoSim_SimulationCallback simulationBeginStepCallback = callbacks.simulationBeginStepCallback;
    const DsVeosCoSim_SimulationCallback simulationEndStepCallback = callbacks.simulationEndStepCallback;
    void* userData = callbacks.userData;

    if (canMessageReceivedCallback) {
        newCallbacks.canMessageReceivedCallback =
            [=](const SimulationTime simulationTime, const CanController& canController, const CanMessage& message) {
                canMessageReceivedCallback(simulationTime.count(),
                                           reinterpret_cast<const DsVeosCoSim_CanController*>(&canController),
                                           reinterpret_cast<const DsVeosCoSim_CanMessage*>(&message),
                                           userData);
            };
    }

    if (ethMessageReceivedCallback) {
        newCallbacks.ethMessageReceivedCallback =
            [=](const SimulationTime simulationTime, const EthController& ethController, const EthMessage& message) {
                ethMessageReceivedCallback(simulationTime.count(),
                                           reinterpret_cast<const DsVeosCoSim_EthController*>(&ethController),
                                           reinterpret_cast<const DsVeosCoSim_EthMessage*>(&message),
                                           userData);
            };
    }

    if (linMessageReceivedCallback) {
        newCallbacks.linMessageReceivedCallback =
            [=](const SimulationTime simulationTime, const LinController& linController, const LinMessage& message) {
                linMessageReceivedCallback(simulationTime.count(),
                                           reinterpret_cast<const DsVeosCoSim_LinController*>(&linController),
                                           reinterpret_cast<const DsVeosCoSim_LinMessage*>(&message),
                                           userData);
            };
    }

    if (incomingSignalChangedCallback) {
        newCallbacks.incomingSignalChangedCallback = [=](const SimulationTime simulationTime,
                                                         const IoSignal& ioSignal,
                                                         const uint32_t length,
                                                         const void* value) {
            incomingSignalChangedCallback(simulationTime.count(),
                                          reinterpret_cast<const DsVeosCoSim_IoSignal*>(&ioSignal),
                                          length,
                                          value,
                                          userData);
        };
    }

    if (simulationStartedCallback) {
        newCallbacks.simulationStartedCallback = [=](const SimulationTime simulationTime) {
            simulationStartedCallback(simulationTime.count(), userData);
        };
    }

    if (simulationStoppedCallback) {
        newCallbacks.simulationStoppedCallback = [=](const SimulationTime simulationTime) {
            simulationStoppedCallback(simulationTime.count(), userData);
        };
    }

    if (simulationPausedCallback) {
        newCallbacks.simulationPausedCallback = [=](const SimulationTime simulationTime) {
            simulationPausedCallback(simulationTime.count(), userData);
        };
    }

    if (simulationContinuedCallback) {
        newCallbacks.simulationContinuedCallback = [=](const SimulationTime simulationTime) {
            simulationContinuedCallback(simulationTime.count(), userData);
        };
    }

    if (simulationTerminatedCallback) {
        newCallbacks.simulationTerminatedCallback = [=](const SimulationTime simulationTime,
                                                        const TerminateReason reason) {
            simulationTerminatedCallback(simulationTime.count(),
                                         static_cast<DsVeosCoSim_TerminateReason>(reason),
                                         userData);
        };
    }

    if (simulationBeginStepCallback) {
        newCallbacks.simulationBeginStepCallback = [=](const SimulationTime simulationTime) {
            simulationBeginStepCallback(simulationTime.count(), userData);
        };
    }

    if (simulationEndStepCallback) {
        newCallbacks.simulationEndStepCallback = [=](const SimulationTime simulationTime) {
            simulationEndStepCallback(simulationTime.count(), userData);
        };
    }
}

}  // namespace

void DsVeosCoSim_SetLogCallback(const DsVeosCoSim_LogCallback logCallback) {
    LogCallbackHandler = logCallback;
    SetLogCallback([](const Severity severity, const std::string_view message) {
        if (LogCallbackHandler) {
            LogCallbackHandler(static_cast<DsVeosCoSim_Severity>(severity), message.data());
        }
    });
}

DsVeosCoSim_Handle DsVeosCoSim_Create() {
    auto client = CreateClient();
    return client.release();
}

void DsVeosCoSim_Destroy(const DsVeosCoSim_Handle handle) {
    if (!handle) {
        return;
    }

    const auto* const client = static_cast<CoSimClient*>(handle);

    delete client;
}

DsVeosCoSim_Result DsVeosCoSim_Connect(const DsVeosCoSim_Handle handle,
                                       const DsVeosCoSim_ConnectConfig connectConfig) {  // NOLINT
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

DsVeosCoSim_Result DsVeosCoSim_Disconnect(const DsVeosCoSim_Handle handle) {
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

DsVeosCoSim_Result DsVeosCoSim_GetConnectionState(const DsVeosCoSim_Handle handle,
                                                  DsVeosCoSim_ConnectionState* connectionState) {
    CheckNotNull(handle);
    CheckNotNull(connectionState);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        *connectionState = static_cast<DsVeosCoSim_ConnectionState>(client->GetConnectionState());

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_RunCallbackBasedCoSimulation(const DsVeosCoSim_Handle handle,
                                                            const DsVeosCoSim_Callbacks callbacks) {  // NOLINT
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    Callbacks newCallbacks{};
    InitializeCallbacks(newCallbacks, callbacks);

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

DsVeosCoSim_Result DsVeosCoSim_StartPollingBasedCoSimulation(const DsVeosCoSim_Handle handle,
                                                             const DsVeosCoSim_Callbacks callbacks) {  // NOLINT
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    Callbacks newCallbacks{};
    InitializeCallbacks(newCallbacks, callbacks);

    try {
        client->StartPollingBasedCoSimulation(newCallbacks);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_PollCommand(const DsVeosCoSim_Handle handle,
                                           DsVeosCoSim_SimulationTime* simulationTime,
                                           DsVeosCoSim_Command* command) {
    CheckNotNull(handle);
    CheckNotNull(simulationTime);
    CheckNotNull(command);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        SimulationTime currentSimulationTime{};
        if (client->PollCommand(currentSimulationTime, *reinterpret_cast<Command*>(command), false)) {
            *simulationTime = currentSimulationTime.count();
            return DsVeosCoSim_Result_Ok;
        }

        return DsVeosCoSim_Result_Disconnected;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_FinishCommand(const DsVeosCoSim_Handle handle) {
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

DsVeosCoSim_Result DsVeosCoSim_SetNextSimulationTime(const DsVeosCoSim_Handle handle,
                                                     const DsVeosCoSim_SimulationTime simulationTime) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->SetNextSimulationTime(SimulationTime(simulationTime));

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetStepSize(const DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime* stepSize) {
    CheckNotNull(handle);
    CheckNotNull(stepSize);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        *stepSize = client->GetStepSize().count();

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetIncomingSignals(const DsVeosCoSim_Handle handle,
                                                  uint32_t* incomingSignalsCount,
                                                  const DsVeosCoSim_IoSignal** incomingSignals) {
    CheckNotNull(handle);
    CheckNotNull(incomingSignalsCount);
    CheckNotNull(incomingSignals);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->GetIncomingSignals(incomingSignalsCount, reinterpret_cast<const IoSignal**>(incomingSignals));

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_ReadIncomingSignal(const DsVeosCoSim_Handle handle,
                                                  const DsVeosCoSim_IoSignalId incomingSignalId,
                                                  uint32_t* length,
                                                  void* value) {
    CheckNotNull(handle);
    CheckNotNull(length);
    CheckNotNull(value);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->Read(static_cast<IoSignalId>(incomingSignalId), *length, value);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetOutgoingSignals(const DsVeosCoSim_Handle handle,
                                                  uint32_t* outgoingSignalsCount,
                                                  const DsVeosCoSim_IoSignal** outgoingSignals) {
    CheckNotNull(handle);
    CheckNotNull(outgoingSignalsCount);
    CheckNotNull(outgoingSignals);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->GetOutgoingSignals(outgoingSignalsCount, reinterpret_cast<const IoSignal**>(outgoingSignals));

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_WriteOutgoingSignal(const DsVeosCoSim_Handle handle,
                                                   const DsVeosCoSim_IoSignalId outgoingSignalId,
                                                   const uint32_t length,
                                                   const void* value) {
    CheckNotNull(handle);
    if (length > 0) {
        CheckNotNull(value);
    }

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->Write(static_cast<IoSignalId>(outgoingSignalId), length, value);

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetCanControllers(const DsVeosCoSim_Handle handle,
                                                 uint32_t* canControllersCount,
                                                 const DsVeosCoSim_CanController** canControllers) {
    CheckNotNull(handle);
    CheckNotNull(canControllersCount);
    CheckNotNull(canControllers);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->GetCanControllers(canControllersCount, reinterpret_cast<const CanController**>(canControllers));

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveCanMessage(const DsVeosCoSim_Handle handle, DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (!client->Receive(reinterpret_cast<CanMessage&>(*message))) {
            return DsVeosCoSim_Result_Empty;
        }

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_TransmitCanMessage(const DsVeosCoSim_Handle handle,
                                                  const DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (!client->Transmit(reinterpret_cast<const CanMessage&>(*message))) {
            return DsVeosCoSim_Result_Full;
        }

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetEthControllers(const DsVeosCoSim_Handle handle,
                                                 uint32_t* ethControllersCount,
                                                 const DsVeosCoSim_EthController** ethControllers) {
    CheckNotNull(handle);
    CheckNotNull(ethControllersCount);
    CheckNotNull(ethControllers);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->GetEthControllers(ethControllersCount, reinterpret_cast<const EthController**>(ethControllers));

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveEthMessage(const DsVeosCoSim_Handle handle, DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (!client->Receive(reinterpret_cast<EthMessage&>(*message))) {
            return DsVeosCoSim_Result_Empty;
        }

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_TransmitEthMessage(const DsVeosCoSim_Handle handle,
                                                  const DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (!client->Transmit(reinterpret_cast<const EthMessage&>(*message))) {
            return DsVeosCoSim_Result_Full;
        }

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetLinControllers(const DsVeosCoSim_Handle handle,
                                                 uint32_t* linControllersCount,
                                                 const DsVeosCoSim_LinController** linControllers) {
    CheckNotNull(handle);
    CheckNotNull(linControllersCount);
    CheckNotNull(linControllers);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->GetLinControllers(linControllersCount, reinterpret_cast<const LinController**>(linControllers));

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveLinMessage(const DsVeosCoSim_Handle handle, DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (!client->Receive(reinterpret_cast<LinMessage&>(*message))) {
            return DsVeosCoSim_Result_Empty;
        }

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_TransmitLinMessage(const DsVeosCoSim_Handle handle,
                                                  const DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        if (!client->Transmit(reinterpret_cast<const LinMessage&>(*message))) {
            return DsVeosCoSim_Result_Full;
        }

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_StartSimulation(const DsVeosCoSim_Handle handle) {
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

DsVeosCoSim_Result DsVeosCoSim_StopSimulation(const DsVeosCoSim_Handle handle) {
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

DsVeosCoSim_Result DsVeosCoSim_PauseSimulation(const DsVeosCoSim_Handle handle) {
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

DsVeosCoSim_Result DsVeosCoSim_ContinueSimulation(const DsVeosCoSim_Handle handle) {
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

DsVeosCoSim_Result DsVeosCoSim_TerminateSimulation(const DsVeosCoSim_Handle handle,
                                                   const DsVeosCoSim_TerminateReason terminateReason) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    try {
        client->Terminate(static_cast<TerminateReason>(terminateReason));

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetCurrentSimulationTime(const DsVeosCoSim_Handle handle,
                                                                         DsVeosCoSim_SimulationTime* simulationTime) {
    CheckNotNull(handle);
    CheckNotNull(simulationTime);

    const auto* const client = static_cast<CoSimClient*>(handle);

    try {
        *simulationTime = client->GetCurrentSimulationTime().count();

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

std::string DsVeosCoSim_SimulationTimeToString(const DsVeosCoSim_SimulationTime simulationTime) {
    return SimulationTimeToString(SimulationTime(simulationTime));
}

std::string DsVeosCoSim_ResultToString(const DsVeosCoSim_Result result) {
    return ToString(static_cast<Result>(result));
}

std::string DsVeosCoSim_CommandToString(DsVeosCoSim_Command command) {
    return ToString(static_cast<Command>(command));
}

std::string DsVeosCoSim_SeverityToString(const DsVeosCoSim_Severity severity) {
    return ToString(static_cast<Severity>(severity));
}

std::string DsVeosCoSim_TerminateReasonToString(const DsVeosCoSim_TerminateReason terminateReason) {
    return ToString(static_cast<TerminateReason>(terminateReason));
}

std::string DsVeosCoSim_ConnectionStateToString(const DsVeosCoSim_ConnectionState connectionState) {
    return ToString(static_cast<ConnectionState>(connectionState));
}

std::string DsVeosCoSim_DataTypeToString(const DsVeosCoSim_DataType dataType) {
    return ToString(static_cast<DataType>(dataType));
}

std::string DsVeosCoSim_SizeKindToString(const DsVeosCoSim_SizeKind sizeKind) {
    return ToString(static_cast<SizeKind>(sizeKind));
}

std::string DsVeosCoSim_IoSignalToString(const DsVeosCoSim_IoSignal& ioSignal) {
    return ToString(reinterpret_cast<const IoSignal&>(ioSignal));
}

std::string DsVeosCoSim_CanControllerToString(const DsVeosCoSim_CanController& controller) {
    return ToString(reinterpret_cast<const CanController&>(controller));
}

std::string DsVeosCoSim_EthControllerToString(const DsVeosCoSim_EthController& controller) {
    return ToString(reinterpret_cast<const EthController&>(controller));
}

std::string DsVeosCoSim_LinControllerToString(const DsVeosCoSim_LinController& controller) {
    return ToString(reinterpret_cast<const LinController&>(controller));
}

std::string DsVeosCoSim_ValueToString(const DsVeosCoSim_DataType dataType, const uint32_t length, const void* value) {
    return ValueToString(static_cast<DataType>(dataType), length, value);
}

std::string DsVeosCoSim_DataToString(const uint8_t* data, const size_t dataLength, const char separator) {
    return DataToString(data, dataLength, separator);
}

std::string DsVeosCoSim_IoDataToString(const DsVeosCoSim_IoSignal& ioSignal, const uint32_t length, const void* value) {
    return IoDataToString(reinterpret_cast<const IoSignal&>(ioSignal), length, value);
}

std::string DsVeosCoSim_CanMessageToString(const DsVeosCoSim_CanMessage& message) {
    return ToString(reinterpret_cast<const CanMessage&>(message));
}

std::string DsVeosCoSim_EthMessageToString(const DsVeosCoSim_EthMessage& message) {
    return ToString(reinterpret_cast<const EthMessage&>(message));
}

std::string DsVeosCoSim_LinMessageToString(const DsVeosCoSim_LinMessage& message) {
    return ToString(reinterpret_cast<const LinMessage&>(message));
}

std::string DsVeosCoSim_LinControllerTypeToString(const DsVeosCoSim_LinControllerType linControllerType) {
    return ToString(static_cast<LinControllerType>(linControllerType));
}

std::string DsVeosCoSim_CanMessageFlagsToString(const DsVeosCoSim_CanMessageFlags flags) {
    return ToString(static_cast<CanMessageFlags>(flags));
}

std::string DsVeosCoSim_EthMessageFlagsToString(const DsVeosCoSim_EthMessageFlags flags) {
    return ToString(static_cast<EthMessageFlags>(flags));
}

std::string DsVeosCoSim_LinMessageFlagsToString(const DsVeosCoSim_LinMessageFlags flags) {
    return ToString(static_cast<LinMessageFlags>(flags));
}

size_t DsVeosCoSim_GetDataTypeSize(const DsVeosCoSim_DataType dataType) {
    return GetDataTypeSize(static_cast<DataType>(dataType));
}
