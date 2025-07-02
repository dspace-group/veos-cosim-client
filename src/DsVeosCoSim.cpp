// Copyright dSPACE GmbH. All rights reserved.

#include "DsVeosCoSim/DsVeosCoSim.h"

#include <cstddef>
#include <cstdint>
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
    std::unique_ptr<CoSimClient> client;
    if (!IsOk(CreateClient(client))) {
        return nullptr;
    }

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

    return static_cast<DsVeosCoSim_Result>(client->Connect(config));
}

DsVeosCoSim_Result DsVeosCoSim_Disconnect(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

    client->Disconnect();

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_GetConnectionState(DsVeosCoSim_Handle handle,
                                                  DsVeosCoSim_ConnectionState* connectionState) {
    CheckNotNull(handle);
    CheckNotNull(connectionState);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(
        client->GetConnectionState(*reinterpret_cast<ConnectionState*>(connectionState)));
}

DsVeosCoSim_Result DsVeosCoSim_RunCallbackBasedCoSimulation(DsVeosCoSim_Handle handle,
                                                            DsVeosCoSim_Callbacks callbacks) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

    Callbacks newCallbacks{};
    InitializeCallbacks(newCallbacks, callbacks);

    return static_cast<DsVeosCoSim_Result>(client->RunCallbackBasedCoSimulation(newCallbacks));
}

DsVeosCoSim_Result DsVeosCoSim_StartPollingBasedCoSimulation(DsVeosCoSim_Handle handle,
                                                             DsVeosCoSim_Callbacks callbacks) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    Callbacks newCallbacks{};
    InitializeCallbacks(newCallbacks, callbacks);

    return static_cast<DsVeosCoSim_Result>(client->StartPollingBasedCoSimulation(newCallbacks));
}

DsVeosCoSim_Result DsVeosCoSim_PollCommand(DsVeosCoSim_Handle handle,
                                           DsVeosCoSim_SimulationTime* simulationTime,
                                           DsVeosCoSim_Command* command) {
    CheckNotNull(handle);
    CheckNotNull(simulationTime);
    CheckNotNull(command);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->PollCommand(*reinterpret_cast<SimulationTime*>(simulationTime),
                                                               *reinterpret_cast<Command*>(command),
                                                               false));
}

DsVeosCoSim_Result DsVeosCoSim_FinishCommand(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->FinishCommand());
}

DsVeosCoSim_Result DsVeosCoSim_SetNextSimulationTime(DsVeosCoSim_Handle handle,
                                                     DsVeosCoSim_SimulationTime simulationTime) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->SetNextSimulationTime(SimulationTime(simulationTime)));
}

DsVeosCoSim_Result DsVeosCoSim_GetStepSize(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime* stepSize) {
    CheckNotNull(handle);
    CheckNotNull(stepSize);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetStepSize(*reinterpret_cast<SimulationTime*>(stepSize)));
}

DsVeosCoSim_Result DsVeosCoSim_GetIncomingSignals(DsVeosCoSim_Handle handle,
                                                  uint32_t* incomingSignalsCount,
                                                  const DsVeosCoSim_IoSignal** incomingSignals) {
    CheckNotNull(handle);
    CheckNotNull(incomingSignalsCount);
    CheckNotNull(incomingSignals);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(
        client->GetIncomingSignals(*incomingSignalsCount, *reinterpret_cast<const IoSignal**>(incomingSignals)));
}

DsVeosCoSim_Result DsVeosCoSim_ReadIncomingSignal(DsVeosCoSim_Handle handle,
                                                  DsVeosCoSim_IoSignalId incomingSignalId,
                                                  uint32_t* length,
                                                  void* value) {
    CheckNotNull(handle);
    CheckNotNull(length);
    CheckNotNull(value);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Read(static_cast<IoSignalId>(incomingSignalId), *length, value));
}

DsVeosCoSim_Result DsVeosCoSim_GetOutgoingSignals(DsVeosCoSim_Handle handle,
                                                  uint32_t* outgoingSignalsCount,
                                                  const DsVeosCoSim_IoSignal** outgoingSignals) {
    CheckNotNull(handle);
    CheckNotNull(outgoingSignalsCount);
    CheckNotNull(outgoingSignals);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(
        client->GetOutgoingSignals(*outgoingSignalsCount, *reinterpret_cast<const IoSignal**>(outgoingSignals)));
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

    return static_cast<DsVeosCoSim_Result>(client->Write(static_cast<IoSignalId>(outgoingSignalId), length, value));
}

DsVeosCoSim_Result DsVeosCoSim_GetCanControllers(DsVeosCoSim_Handle handle,
                                                 uint32_t* canControllersCount,
                                                 const DsVeosCoSim_CanController** canControllers) {
    CheckNotNull(handle);
    CheckNotNull(canControllersCount);
    CheckNotNull(canControllers);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(
        client->GetCanControllers(*canControllersCount, *reinterpret_cast<const CanController**>(canControllers)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveCanMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(reinterpret_cast<CanMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitCanMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(reinterpret_cast<const CanMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_GetEthControllers(DsVeosCoSim_Handle handle,
                                                 uint32_t* ethControllersCount,
                                                 const DsVeosCoSim_EthController** ethControllers) {
    CheckNotNull(handle);
    CheckNotNull(ethControllersCount);
    CheckNotNull(ethControllers);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(
        client->GetEthControllers(*ethControllersCount, *reinterpret_cast<const EthController**>(ethControllers)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveEthMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(reinterpret_cast<EthMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitEthMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(reinterpret_cast<const EthMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_GetLinControllers(DsVeosCoSim_Handle handle,
                                                 uint32_t* linControllersCount,
                                                 const DsVeosCoSim_LinController** linControllers) {
    CheckNotNull(handle);
    CheckNotNull(linControllersCount);
    CheckNotNull(linControllers);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(
        client->GetEthControllers(*linControllersCount, *reinterpret_cast<const EthController**>(linControllers)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveLinMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(reinterpret_cast<LinMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitLinMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(reinterpret_cast<const LinMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_StartSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Start());
}

DsVeosCoSim_Result DsVeosCoSim_StopSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Stop());
}

DsVeosCoSim_Result DsVeosCoSim_PauseSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Pause());
}

DsVeosCoSim_Result DsVeosCoSim_ContinueSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Continue());
}

DsVeosCoSim_Result DsVeosCoSim_TerminateSimulation(DsVeosCoSim_Handle handle,
                                                   DsVeosCoSim_TerminateReason terminateReason) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Terminate(static_cast<TerminateReason>(terminateReason)));
}

DsVeosCoSim_Result DsVeosCoSim_GetCurrentSimulationTime(DsVeosCoSim_Handle handle,
                                                        DsVeosCoSim_SimulationTime* simulationTime) {
    CheckNotNull(handle);
    CheckNotNull(simulationTime);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(
        client->GetCurrentSimulationTime(*reinterpret_cast<SimulationTime*>(simulationTime)));
}

const char* DsVeosCoSim_ResultToString(DsVeosCoSim_Result result) {
    return ToString(static_cast<Result>(result)).data();
}

const char* DsVeosCoSim_CommandToString(DsVeosCoSim_Command command) {
    return ToString(static_cast<Command>(command)).data();
}

const char* DsVeosCoSim_SeverityToString(DsVeosCoSim_Severity severity) {
    return ToString(static_cast<Severity>(severity)).data();
}

const char* DsVeosCoSim_TerminateReasonToString(DsVeosCoSim_TerminateReason terminateReason) {
    return ToString(static_cast<TerminateReason>(terminateReason)).data();
}

const char* DsVeosCoSim_ConnectionStateToString(DsVeosCoSim_ConnectionState connectionState) {
    return ToString(static_cast<ConnectionState>(connectionState)).data();
}

const char* DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType dataType) {
    return ToString(static_cast<DataType>(dataType)).data();
}

const char* DsVeosCoSim_SizeKindToString(DsVeosCoSim_SizeKind sizeKind) {
    return ToString(static_cast<SizeKind>(sizeKind)).data();
}

const char* DsVeosCoSim_LinControllerTypeToString(DsVeosCoSim_LinControllerType linControllerType) {
    return ToString(static_cast<LinControllerType>(linControllerType)).data();
}

size_t DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType dataType) {
    return GetDataTypeSize(static_cast<DataType>(dataType));
}

std::string DsVeosCoSim_SimulationTimeToString(DsVeosCoSim_SimulationTime simulationTime) {
    return SimulationTimeToString(SimulationTime(simulationTime));
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

std::string DsVeosCoSim_CanMessageFlagsToString(DsVeosCoSim_CanMessageFlags flags) {
    return ToString(static_cast<CanMessageFlags>(flags));
}

std::string DsVeosCoSim_EthMessageFlagsToString(DsVeosCoSim_EthMessageFlags flags) {
    return ToString(static_cast<EthMessageFlags>(flags));
}

std::string DsVeosCoSim_LinMessageFlagsToString(DsVeosCoSim_LinMessageFlags flags) {
    return ToString(static_cast<LinMessageFlags>(flags));
}
