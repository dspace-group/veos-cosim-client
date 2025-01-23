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

DsVeosCoSim_LogCallback LogCallbackHandler;

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
    auto client = std::make_unique<CoSimClient>();
    return client.release();
}

void DsVeosCoSim_Destroy(const DsVeosCoSim_Handle handle) {
    if (!handle) {
        return;
    }

    const auto* const client = static_cast<CoSimClient*>(handle);

    delete client;
}

DsVeosCoSim_Result DsVeosCoSim_Connect(DsVeosCoSim_Handle handle, DsVeosCoSim_ConnectConfig connectConfig) {  // NOLINT
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
                                                            DsVeosCoSim_Callbacks callbacks) {  // NOLINT
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

DsVeosCoSim_Result DsVeosCoSim_StartPollingBasedCoSimulation(const DsVeosCoSim_Handle handle,
                                                             DsVeosCoSim_Callbacks callbacks) {  // NOLINT
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

std::string DsVeosCoSim_IoDataToString(const DsVeosCoSim_SimulationTime simulationTime,
                                       const DsVeosCoSim_IoSignal& ioSignal,
                                       const uint32_t length,
                                       const void* value) {
    return IoDataToString(SimulationTime(simulationTime), reinterpret_cast<const IoSignal&>(ioSignal), length, value);
}

std::string DsVeosCoSim_CanMessageToString(const DsVeosCoSim_SimulationTime simulationTime,
                                           const DsVeosCoSim_CanController& controller,
                                           const DsVeosCoSim_CanMessage& message) {
    return CanMessageToString(SimulationTime(simulationTime),
                              reinterpret_cast<const CanController&>(controller),
                              reinterpret_cast<const CanMessage&>(message));
}

std::string DsVeosCoSim_EthMessageToString(const DsVeosCoSim_SimulationTime simulationTime,
                                           const DsVeosCoSim_EthController& controller,
                                           const DsVeosCoSim_EthMessage& message) {
    return EthMessageToString(SimulationTime(simulationTime),
                              reinterpret_cast<const EthController&>(controller),
                              reinterpret_cast<const EthMessage&>(message));
}

std::string DsVeosCoSim_LinMessageToString(const DsVeosCoSim_SimulationTime simulationTime,
                                           const DsVeosCoSim_LinController& controller,
                                           const DsVeosCoSim_LinMessage& message) {
    return LinMessageToString(SimulationTime(simulationTime),
                              reinterpret_cast<const LinController&>(controller),
                              reinterpret_cast<const LinMessage&>(message));
}

std::string DsVeosCoSim_LinControllerTypeToString(const DsVeosCoSim_LinControllerType linControllerType) {
    return ToString(static_cast<LinControllerType>(linControllerType));
}

std::string DsVeosCoSim_CanMessageFlagsToString(const DsVeosCoSim_CanMessageFlags flags) {
    return CanMessageFlagsToString(static_cast<CanMessageFlags>(flags));
}

std::string DsVeosCoSim_EthMessageFlagsToString(const DsVeosCoSim_EthMessageFlags flags) {
    return EthMessageFlagsToString(static_cast<EthMessageFlags>(flags));
}

std::string DsVeosCoSim_LinMessageFlagsToString(const DsVeosCoSim_LinMessageFlags flags) {
    return LinMessageFlagsToString(static_cast<LinMessageFlags>(flags));
}

size_t DsVeosCoSim_GetDataTypeSize(const DsVeosCoSim_DataType dataType) {
    return GetDataTypeSize(static_cast<DataType>(dataType));
}
