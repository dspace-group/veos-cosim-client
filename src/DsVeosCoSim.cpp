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

void InitializeCallbacks(Callbacks& newCallbacks, const DsVeosCoSim_Callbacks& callbacks) {
    DsVeosCoSim_CanMessageReceivedCallback canMessageReceivedCallback = callbacks.canMessageReceivedCallback;
    DsVeosCoSim_EthMessageReceivedCallback ethMessageReceivedCallback = callbacks.ethMessageReceivedCallback;
    DsVeosCoSim_LinMessageReceivedCallback linMessageReceivedCallback = callbacks.linMessageReceivedCallback;
    DsVeosCoSim_IncomingSignalChangedCallback incomingSignalChangedCallback = callbacks.incomingSignalChangedCallback;
    DsVeosCoSim_SimulationCallback simulationStartedCallback = callbacks.simulationStartedCallback;
    DsVeosCoSim_SimulationCallback simulationStoppedCallback = callbacks.simulationStoppedCallback;
    DsVeosCoSim_SimulationCallback simulationPausedCallback = callbacks.simulationPausedCallback;
    DsVeosCoSim_SimulationCallback simulationContinuedCallback = callbacks.simulationContinuedCallback;
    DsVeosCoSim_SimulationTerminatedCallback simulationTerminatedCallback = callbacks.simulationTerminatedCallback;
    DsVeosCoSim_SimulationCallback simulationBeginStepCallback = callbacks.simulationBeginStepCallback;
    DsVeosCoSim_SimulationCallback simulationEndStepCallback = callbacks.simulationEndStepCallback;
    void* userData = callbacks.userData;

    if (canMessageReceivedCallback) {
        newCallbacks.canMessageReceivedCallback =
            [=](SimulationTime simulationTime, const CanController& canController, const CanMessage& message) {
                canMessageReceivedCallback(simulationTime.count(),
                                           reinterpret_cast<const DsVeosCoSim_CanController*>(&canController),
                                           reinterpret_cast<const DsVeosCoSim_CanMessage*>(&message),
                                           userData);
            };
    }

    if (ethMessageReceivedCallback) {
        newCallbacks.ethMessageReceivedCallback =
            [=](SimulationTime simulationTime, const EthController& ethController, const EthMessage& message) {
                ethMessageReceivedCallback(simulationTime.count(),
                                           reinterpret_cast<const DsVeosCoSim_EthController*>(&ethController),
                                           reinterpret_cast<const DsVeosCoSim_EthMessage*>(&message),
                                           userData);
            };
    }

    if (linMessageReceivedCallback) {
        newCallbacks.linMessageReceivedCallback =
            [=](SimulationTime simulationTime, const LinController& linController, const LinMessage& message) {
                linMessageReceivedCallback(simulationTime.count(),
                                           reinterpret_cast<const DsVeosCoSim_LinController*>(&linController),
                                           reinterpret_cast<const DsVeosCoSim_LinMessage*>(&message),
                                           userData);
            };
    }

    if (incomingSignalChangedCallback) {
        newCallbacks.incomingSignalChangedCallback =
            [=](SimulationTime simulationTime, const IoSignal& ioSignal, uint32_t length, const void* value) {
                incomingSignalChangedCallback(simulationTime.count(),
                                              reinterpret_cast<const DsVeosCoSim_IoSignal*>(&ioSignal),
                                              length,
                                              value,
                                              userData);
            };
    }

    if (simulationStartedCallback) {
        newCallbacks.simulationStartedCallback = [=](SimulationTime simulationTime) {
            simulationStartedCallback(simulationTime.count(), userData);
        };
    }

    if (simulationStoppedCallback) {
        newCallbacks.simulationStoppedCallback = [=](SimulationTime simulationTime) {
            simulationStoppedCallback(simulationTime.count(), userData);
        };
    }

    if (simulationPausedCallback) {
        newCallbacks.simulationPausedCallback = [=](SimulationTime simulationTime) {
            simulationPausedCallback(simulationTime.count(), userData);
        };
    }

    if (simulationContinuedCallback) {
        newCallbacks.simulationContinuedCallback = [=](SimulationTime simulationTime) {
            simulationContinuedCallback(simulationTime.count(), userData);
        };
    }

    if (simulationTerminatedCallback) {
        newCallbacks.simulationTerminatedCallback = [=](SimulationTime simulationTime, TerminateReason reason) {
            simulationTerminatedCallback(simulationTime.count(),
                                         static_cast<DsVeosCoSim_TerminateReason>(reason),
                                         userData);
        };
    }

    if (simulationBeginStepCallback) {
        newCallbacks.simulationBeginStepCallback = [=](SimulationTime simulationTime) {
            simulationBeginStepCallback(simulationTime.count(), userData);
        };
    }

    if (simulationEndStepCallback) {
        newCallbacks.simulationEndStepCallback = [=](SimulationTime simulationTime) {
            simulationEndStepCallback(simulationTime.count(), userData);
        };
    }
}

}  // namespace

void DsVeosCoSim_SetLogCallback(DsVeosCoSim_LogCallback logCallback) {
    SetLogCallback([=](Severity severity, std::string_view message) {
        if (logCallback) {
            logCallback(static_cast<DsVeosCoSim_Severity>(severity), message.data());
        }
    });
}

DsVeosCoSim_Handle DsVeosCoSim_Create() {
    auto client = CreateClient();
    return client.release();
}

void DsVeosCoSim_Destroy(DsVeosCoSim_Handle handle) {
    if (!handle) {
        return;
    }

    auto* client = static_cast<CoSimClient*>(handle);

    delete client;
}

DsVeosCoSim_Result DsVeosCoSim_Connect(DsVeosCoSim_Handle handle, DsVeosCoSim_ConnectConfig connectConfig) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

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

    auto* client = static_cast<CoSimClient*>(handle);

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

    auto* client = static_cast<CoSimClient*>(handle);

    try {
        *connectionState = static_cast<DsVeosCoSim_ConnectionState>(client->GetConnectionState());

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_RunCallbackBasedCoSimulation(DsVeosCoSim_Handle handle,
                                                            DsVeosCoSim_Callbacks callbacks) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

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

DsVeosCoSim_Result DsVeosCoSim_StartPollingBasedCoSimulation(DsVeosCoSim_Handle handle,
                                                             DsVeosCoSim_Callbacks callbacks) {
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

DsVeosCoSim_Result DsVeosCoSim_PollCommand(DsVeosCoSim_Handle handle,
                                           DsVeosCoSim_SimulationTime* simulationTime,
                                           DsVeosCoSim_Command* command) {
    CheckNotNull(handle);
    CheckNotNull(simulationTime);
    CheckNotNull(command);

    auto* client = static_cast<CoSimClient*>(handle);

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

DsVeosCoSim_Result DsVeosCoSim_FinishCommand(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

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

    auto* client = static_cast<CoSimClient*>(handle);

    try {
        client->SetNextSimulationTime(SimulationTime(simulationTime));

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_GetStepSize(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime* stepSize) {
    CheckNotNull(handle);
    CheckNotNull(stepSize);

    auto* client = static_cast<CoSimClient*>(handle);

    try {
        *stepSize = client->GetStepSize().count();

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

    auto* client = static_cast<CoSimClient*>(handle);

    try {
        client->GetIncomingSignals(incomingSignalsCount, reinterpret_cast<const IoSignal**>(incomingSignals));

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

    auto* client = static_cast<CoSimClient*>(handle);

    try {
        client->Read(static_cast<IoSignalId>(incomingSignalId), *length, value);

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

    auto* client = static_cast<CoSimClient*>(handle);

    try {
        client->GetOutgoingSignals(outgoingSignalsCount, reinterpret_cast<const IoSignal**>(outgoingSignals));

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

    auto* client = static_cast<CoSimClient*>(handle);

    try {
        client->Write(static_cast<IoSignalId>(outgoingSignalId), length, value);

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

    auto* client = static_cast<CoSimClient*>(handle);

    try {
        client->GetCanControllers(canControllersCount, reinterpret_cast<const CanController**>(canControllers));

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveCanMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

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

DsVeosCoSim_Result DsVeosCoSim_TransmitCanMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

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

DsVeosCoSim_Result DsVeosCoSim_GetEthControllers(DsVeosCoSim_Handle handle,
                                                 uint32_t* ethControllersCount,
                                                 const DsVeosCoSim_EthController** ethControllers) {
    CheckNotNull(handle);
    CheckNotNull(ethControllersCount);
    CheckNotNull(ethControllers);

    auto* client = static_cast<CoSimClient*>(handle);

    try {
        client->GetEthControllers(ethControllersCount, reinterpret_cast<const EthController**>(ethControllers));

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveEthMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

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

DsVeosCoSim_Result DsVeosCoSim_TransmitEthMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

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

DsVeosCoSim_Result DsVeosCoSim_GetLinControllers(DsVeosCoSim_Handle handle,
                                                 uint32_t* linControllersCount,
                                                 const DsVeosCoSim_LinController** linControllers) {
    CheckNotNull(handle);
    CheckNotNull(linControllersCount);
    CheckNotNull(linControllers);

    auto* client = static_cast<CoSimClient*>(handle);

    try {
        client->GetLinControllers(linControllersCount, reinterpret_cast<const LinController**>(linControllers));

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveLinMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

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

DsVeosCoSim_Result DsVeosCoSim_TransmitLinMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

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

DsVeosCoSim_Result DsVeosCoSim_StartSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

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

    auto* client = static_cast<CoSimClient*>(handle);

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

    auto* client = static_cast<CoSimClient*>(handle);

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

    auto* client = static_cast<CoSimClient*>(handle);

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

    auto* client = static_cast<CoSimClient*>(handle);

    try {
        client->Terminate(static_cast<TerminateReason>(terminateReason));

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetCurrentSimulationTime(DsVeosCoSim_Handle handle,
                                                                         DsVeosCoSim_SimulationTime* simulationTime) {
    CheckNotNull(handle);
    CheckNotNull(simulationTime);

    auto* client = static_cast<CoSimClient*>(handle);

    try {
        *simulationTime = client->GetCurrentSimulationTime().count();

        return DsVeosCoSim_Result_Ok;
    } catch (const std::exception& e) {
        LogError(e.what());

        return DsVeosCoSim_Result_Error;
    }
}

std::string DsVeosCoSim_SimulationTimeToString(DsVeosCoSim_SimulationTime simulationTime) {
    return SimulationTimeToString(SimulationTime(simulationTime));
}

std::string_view DsVeosCoSim_ResultToString(DsVeosCoSim_Result result) noexcept {
    return ToString(static_cast<Result>(result));
}

std::string_view DsVeosCoSim_CommandToString(DsVeosCoSim_Command command) noexcept {
    return ToString(static_cast<Command>(command));
}

std::string_view DsVeosCoSim_SeverityToString(DsVeosCoSim_Severity severity) noexcept {
    return ToString(static_cast<Severity>(severity));
}

std::string_view DsVeosCoSim_TerminateReasonToString(DsVeosCoSim_TerminateReason terminateReason) noexcept {
    return ToString(static_cast<TerminateReason>(terminateReason));
}

std::string_view DsVeosCoSim_ConnectionStateToString(DsVeosCoSim_ConnectionState connectionState) noexcept {
    return ToString(static_cast<ConnectionState>(connectionState));
}

std::string_view DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType dataType) noexcept {
    return ToString(static_cast<DataType>(dataType));
}

std::string_view DsVeosCoSim_SizeKindToString(DsVeosCoSim_SizeKind sizeKind) noexcept {
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

std::string DsVeosCoSim_ValueToString(DsVeosCoSim_DataType dataType, uint32_t length, const void* value) {
    return ValueToString(static_cast<DataType>(dataType), length, value);
}

std::string DsVeosCoSim_DataToString(const uint8_t* data, size_t dataLength, char separator) {
    return DataToString(data, dataLength, separator);
}

std::string DsVeosCoSim_IoDataToString(const DsVeosCoSim_IoSignal& ioSignal, uint32_t length, const void* value) {
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

std::string_view DsVeosCoSim_LinControllerTypeToString(DsVeosCoSim_LinControllerType linControllerType) noexcept {
    return ToString(static_cast<LinControllerType>(linControllerType));
}

std::string DsVeosCoSim_CanMessageFlagsToString(DsVeosCoSim_CanMessageFlags flags) {
    return ToString(static_cast<CanMessageFlags>(flags));
}

std::string DsVeosCoSim_EthMessageFlagsToString(DsVeosCoSim_EthMessageFlags flags) {
    return ToString(static_cast<EthMessageFlags>(flags));
}

std::string DsVeosCoSim_LinMessageFlagsToString(DsVeosCoSim_LinMessageFlags flags) {
    return ToString(static_cast<LinMessageFlags>(flags));
}

size_t DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType dataType) noexcept {
    return GetDataTypeSize(static_cast<DataType>(dataType));
}
