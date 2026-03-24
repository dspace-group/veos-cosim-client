// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "DsVeosCoSim/DsVeosCoSim.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include <fmt/format.h>

#include "CoSimClient.hpp"
#include "CoSimTypes.hpp"
#include "Logger.hpp"
#include "Result.hpp"

using namespace DsVeosCoSim;

namespace {

#define CheckNotNull(arg)                                    \
    do {                                                     \
        if (!(arg)) {                                        \
            LogError("Argument " #arg " must not be null."); \
            return DsVeosCoSim_Result_InvalidArgument;       \
        }                                                    \
    } while (0)

[[nodiscard]] const DsVeosCoSim_CanController* Convert(const CanController* controller) {
    return reinterpret_cast<const DsVeosCoSim_CanController*>(controller);
}

[[nodiscard]] const CanController* Convert(const DsVeosCoSim_CanController* controller) {
    return reinterpret_cast<const CanController*>(controller);
}

[[nodiscard]] const CanController** Convert(const DsVeosCoSim_CanController** controllers) {
    return reinterpret_cast<const CanController**>(controllers);
}

[[nodiscard]] const DsVeosCoSim_CanMessage* Convert(const CanMessage* message) {
    return reinterpret_cast<const DsVeosCoSim_CanMessage*>(message);
}

[[nodiscard]] const CanMessage* Convert(const DsVeosCoSim_CanMessage* message) {
    return reinterpret_cast<const CanMessage*>(message);
}

[[nodiscard]] CanMessage* Convert(DsVeosCoSim_CanMessage* message) {
    return reinterpret_cast<CanMessage*>(message);
}

[[nodiscard]] const DsVeosCoSim_CanMessageContainer* Convert(const CanMessageContainer* messageContainer) {
    return reinterpret_cast<const DsVeosCoSim_CanMessageContainer*>(messageContainer);
}

[[nodiscard]] const CanMessageContainer* Convert(const DsVeosCoSim_CanMessageContainer* messageContainer) {
    return reinterpret_cast<const CanMessageContainer*>(messageContainer);
}

[[nodiscard]] CanMessageContainer* Convert(DsVeosCoSim_CanMessageContainer* messageContainer) {
    return reinterpret_cast<CanMessageContainer*>(messageContainer);
}

[[nodiscard]] const DsVeosCoSim_EthController* Convert(const EthController* controller) {
    return reinterpret_cast<const DsVeosCoSim_EthController*>(controller);
}

[[nodiscard]] const EthController** Convert(const DsVeosCoSim_EthController** controllers) {
    return reinterpret_cast<const EthController**>(controllers);
}

[[nodiscard]] const EthController* Convert(const DsVeosCoSim_EthController* controller) {
    return reinterpret_cast<const EthController*>(controller);
}

[[nodiscard]] const DsVeosCoSim_EthMessage* Convert(const EthMessage* message) {
    return reinterpret_cast<const DsVeosCoSim_EthMessage*>(message);
}

[[nodiscard]] const EthMessage* Convert(const DsVeosCoSim_EthMessage* message) {
    return reinterpret_cast<const EthMessage*>(message);
}

[[nodiscard]] EthMessage* Convert(DsVeosCoSim_EthMessage* message) {
    return reinterpret_cast<EthMessage*>(message);
}

[[nodiscard]] const DsVeosCoSim_EthMessageContainer* Convert(const EthMessageContainer* messageContainer) {
    return reinterpret_cast<const DsVeosCoSim_EthMessageContainer*>(messageContainer);
}

[[nodiscard]] const EthMessageContainer* Convert(const DsVeosCoSim_EthMessageContainer* messageContainer) {
    return reinterpret_cast<const EthMessageContainer*>(messageContainer);
}

[[nodiscard]] EthMessageContainer* Convert(DsVeosCoSim_EthMessageContainer* messageContainer) {
    return reinterpret_cast<EthMessageContainer*>(messageContainer);
}

[[nodiscard]] const DsVeosCoSim_LinController* Convert(const LinController* controller) {
    return reinterpret_cast<const DsVeosCoSim_LinController*>(controller);
}

[[nodiscard]] const LinController* Convert(const DsVeosCoSim_LinController* controller) {
    return reinterpret_cast<const LinController*>(controller);
}

[[nodiscard]] const LinController** Convert(const DsVeosCoSim_LinController** controllers) {
    return reinterpret_cast<const LinController**>(controllers);
}

[[nodiscard]] const DsVeosCoSim_LinMessage* Convert(const LinMessage* message) {
    return reinterpret_cast<const DsVeosCoSim_LinMessage*>(message);
}

[[nodiscard]] const LinMessage* Convert(const DsVeosCoSim_LinMessage* message) {
    return reinterpret_cast<const LinMessage*>(message);
}

[[nodiscard]] LinMessage* Convert(DsVeosCoSim_LinMessage* message) {
    return reinterpret_cast<LinMessage*>(message);
}

[[nodiscard]] const DsVeosCoSim_LinMessageContainer* Convert(const LinMessageContainer* messageContainer) {
    return reinterpret_cast<const DsVeosCoSim_LinMessageContainer*>(messageContainer);
}

[[nodiscard]] const LinMessageContainer* Convert(const DsVeosCoSim_LinMessageContainer* messageContainer) {
    return reinterpret_cast<const LinMessageContainer*>(messageContainer);
}

[[nodiscard]] LinMessageContainer* Convert(DsVeosCoSim_LinMessageContainer* messageContainer) {
    return reinterpret_cast<LinMessageContainer*>(messageContainer);
}

[[nodiscard]] const DsVeosCoSim_FrController* Convert(const FrController* controller) {
    return reinterpret_cast<const DsVeosCoSim_FrController*>(controller);
}

[[nodiscard]] const FrController* Convert(const DsVeosCoSim_FrController* controller) {
    return reinterpret_cast<const FrController*>(controller);
}

[[nodiscard]] const FrController** Convert(const DsVeosCoSim_FrController** controllers) {
    return reinterpret_cast<const FrController**>(controllers);
}

[[nodiscard]] const DsVeosCoSim_FrMessage* Convert(const FrMessage* message) {
    return reinterpret_cast<const DsVeosCoSim_FrMessage*>(message);
}

[[nodiscard]] const FrMessage* Convert(const DsVeosCoSim_FrMessage* message) {
    return reinterpret_cast<const FrMessage*>(message);
}

[[nodiscard]] FrMessage* Convert(DsVeosCoSim_FrMessage* message) {
    return reinterpret_cast<FrMessage*>(message);
}

[[nodiscard]] const DsVeosCoSim_FrMessageContainer* Convert(const FrMessageContainer* messageContainer) {
    return reinterpret_cast<const DsVeosCoSim_FrMessageContainer*>(messageContainer);
}

[[nodiscard]] const FrMessageContainer* Convert(const DsVeosCoSim_FrMessageContainer* messageContainer) {
    return reinterpret_cast<const FrMessageContainer*>(messageContainer);
}

[[nodiscard]] FrMessageContainer* Convert(DsVeosCoSim_FrMessageContainer* messageContainer) {
    return reinterpret_cast<FrMessageContainer*>(messageContainer);
}

[[nodiscard]] const DsVeosCoSim_IoSignal* Convert(const IoSignal* ioSignal) {
    return reinterpret_cast<const DsVeosCoSim_IoSignal*>(ioSignal);
}

[[nodiscard]] const IoSignal* Convert(const DsVeosCoSim_IoSignal* ioSignal) {
    return reinterpret_cast<const IoSignal*>(ioSignal);
}

[[nodiscard]] const IoSignal** Convert(const DsVeosCoSim_IoSignal** ioSignals) {
    return reinterpret_cast<const IoSignal**>(ioSignals);
}

[[nodiscard]] constexpr DsVeosCoSim_TerminateReason Convert(TerminateReason terminateReason) {
    return static_cast<DsVeosCoSim_TerminateReason>(terminateReason);
}

[[nodiscard]] constexpr TerminateReason Convert(DsVeosCoSim_TerminateReason terminateReason) {
    return static_cast<TerminateReason>(terminateReason);
}

[[nodiscard]] constexpr DsVeosCoSim_Result Convert(Result result) {
    return static_cast<DsVeosCoSim_Result>(result);
}

[[nodiscard]] constexpr Result Convert(DsVeosCoSim_Result result) {
    return static_cast<Result>(result);
}

[[nodiscard]] constexpr DsVeosCoSim_Severity Convert(Severity severity) {
    return static_cast<DsVeosCoSim_Severity>(severity);
}

[[nodiscard]] constexpr Severity Convert(DsVeosCoSim_Severity severity) {
    return static_cast<Severity>(severity);
}

[[nodiscard]] constexpr CoSimClient* Convert(DsVeosCoSim_Handle handle) {
    return static_cast<CoSimClient*>(handle);
}

[[nodiscard]] ConnectionState* Convert(DsVeosCoSim_ConnectionState* connectionState) {
    return reinterpret_cast<ConnectionState*>(connectionState);
}

[[nodiscard]] SimulationTime* Convert(DsVeosCoSim_SimulationTime* simulationTime) {
    return reinterpret_cast<SimulationTime*>(simulationTime);
}

[[nodiscard]] constexpr Command Convert(DsVeosCoSim_Command command) {
    return static_cast<Command>(command);
}

[[nodiscard]] Command* Convert(DsVeosCoSim_Command* command) {
    return reinterpret_cast<Command*>(command);
}

[[nodiscard]] constexpr IoSignalId Convert(DsVeosCoSim_IoSignalId ioSignalId) {
    return static_cast<IoSignalId>(ioSignalId);
}

[[nodiscard]] constexpr SimulationState Convert(DsVeosCoSim_SimulationState simulationState) {
    return static_cast<SimulationState>(simulationState);
}

[[nodiscard]] SimulationState* Convert(DsVeosCoSim_SimulationState* simulationState) {
    return reinterpret_cast<SimulationState*>(simulationState);
}

[[nodiscard]] constexpr DataType Convert(DsVeosCoSim_DataType dataType) {
    return static_cast<DataType>(dataType);
}

[[nodiscard]] constexpr ConnectionState Convert(DsVeosCoSim_ConnectionState connectionState) {
    return static_cast<ConnectionState>(connectionState);
}

[[nodiscard]] constexpr SizeKind Convert(DsVeosCoSim_SizeKind sizeKind) {
    return static_cast<SizeKind>(sizeKind);
}

[[nodiscard]] constexpr LinControllerType Convert(DsVeosCoSim_LinControllerType linControllerType) {
    return static_cast<LinControllerType>(linControllerType);
}

[[nodiscard]] constexpr CanMessageFlags ConvertCanMessageFlags(DsVeosCoSim_CanMessageFlags flags) {
    return static_cast<CanMessageFlags>(flags);
}

[[nodiscard]] constexpr EthMessageFlags ConvertEthMessageFlags(DsVeosCoSim_EthMessageFlags flags) {
    return static_cast<EthMessageFlags>(flags);
}

[[nodiscard]] constexpr LinMessageFlags ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags flags) {
    return static_cast<LinMessageFlags>(flags);
}

[[nodiscard]] constexpr FrMessageFlags ConvertFrMessageFlags(DsVeosCoSim_FrMessageFlags flags) {
    return static_cast<FrMessageFlags>(flags);
}

void InitializeCallbacks(Callbacks& newCallbacks, const DsVeosCoSim_Callbacks& callbacks) {
    DsVeosCoSim_CanMessageReceivedCallback canMessageReceivedCallback = callbacks.canMessageReceivedCallback;
    DsVeosCoSim_CanMessageContainerReceivedCallback canMessageContainerReceivedCallback = callbacks.canMessageContainerReceivedCallback;
    DsVeosCoSim_EthMessageReceivedCallback ethMessageReceivedCallback = callbacks.ethMessageReceivedCallback;
    DsVeosCoSim_EthMessageContainerReceivedCallback ethMessageContainerReceivedCallback = callbacks.ethMessageContainerReceivedCallback;
    DsVeosCoSim_LinMessageReceivedCallback linMessageReceivedCallback = callbacks.linMessageReceivedCallback;
    DsVeosCoSim_LinMessageContainerReceivedCallback linMessageContainerReceivedCallback = callbacks.linMessageContainerReceivedCallback;
    DsVeosCoSim_FrMessageReceivedCallback frMessageReceivedCallback = callbacks.frMessageReceivedCallback;
    DsVeosCoSim_FrMessageContainerReceivedCallback frMessageContainerReceivedCallback = callbacks.frMessageContainerReceivedCallback;
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
        newCallbacks.canMessageReceivedCallback = [=](SimulationTime simulationTime, const CanController& controller, const CanMessage& message) {
            canMessageReceivedCallback(simulationTime.count(), Convert(&controller), Convert(&message), userData);
        };
    }

    if (canMessageContainerReceivedCallback) {
        newCallbacks.canMessageContainerReceivedCallback =
            [=](SimulationTime simulationTime, const CanController& canController, const CanMessageContainer& messageContainer) {
                canMessageContainerReceivedCallback(simulationTime.count(), Convert(&canController), Convert(&messageContainer), userData);
            };
    }

    if (ethMessageReceivedCallback) {
        newCallbacks.ethMessageReceivedCallback = [=](SimulationTime simulationTime, const EthController& controller, const EthMessage& message) {
            ethMessageReceivedCallback(simulationTime.count(), Convert(&controller), Convert(&message), userData);
        };
    }

    if (ethMessageContainerReceivedCallback) {
        newCallbacks.ethMessageContainerReceivedCallback =
            [=](SimulationTime simulationTime, const EthController& controller, const EthMessageContainer& messageContainer) {
                ethMessageContainerReceivedCallback(simulationTime.count(), Convert(&controller), Convert(&messageContainer), userData);
            };
    }

    if (linMessageReceivedCallback) {
        newCallbacks.linMessageReceivedCallback = [=](SimulationTime simulationTime, const LinController& linController, const LinMessage& message) {
            linMessageReceivedCallback(simulationTime.count(), Convert(&linController), Convert(&message), userData);
        };
    }

    if (linMessageContainerReceivedCallback) {
        newCallbacks.linMessageContainerReceivedCallback =
            [=](SimulationTime simulationTime, const LinController& controller, const LinMessageContainer& messageContainer) {
                linMessageContainerReceivedCallback(simulationTime.count(), Convert(&controller), Convert(&messageContainer), userData);
            };
    }

    if (frMessageReceivedCallback) {
        newCallbacks.frMessageReceivedCallback = [=](SimulationTime simulationTime, const FrController& controller, const FrMessage& message) {
            frMessageReceivedCallback(simulationTime.count(), Convert(&controller), Convert(&message), userData);
        };
    }

    if (frMessageContainerReceivedCallback) {
        newCallbacks.frMessageContainerReceivedCallback =
            [=](SimulationTime simulationTime, const FrController& controller, const FrMessageContainer& messageContainer) {
                frMessageContainerReceivedCallback(simulationTime.count(), Convert(&controller), Convert(&messageContainer), userData);
            };
    }

    if (incomingSignalChangedCallback) {
        newCallbacks.incomingSignalChangedCallback = [=](SimulationTime simulationTime, const IoSignal& ioSignal, uint32_t length, const void* value) {
            incomingSignalChangedCallback(simulationTime.count(), Convert(&ioSignal), length, value, userData);
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
            simulationTerminatedCallback(simulationTime.count(), Convert(reason), userData);
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
    Logger::Instance().SetLogCallback([=](Severity severity, const std::string& message) {
        if (logCallback) {
            logCallback(Convert(severity), message.c_str());
        }
    });
}

DsVeosCoSim_Handle DsVeosCoSim_Create() {
    std::unique_ptr<CoSimClient> client = CreateClient();
    return client.release();
}

void DsVeosCoSim_Destroy(DsVeosCoSim_Handle handle) {
    if (!handle) {
        return;
    }

    CoSimClient* client = Convert(handle);

    delete client;
}

DsVeosCoSim_Result DsVeosCoSim_Connect(DsVeosCoSim_Handle handle, DsVeosCoSim_ConnectConfig connectConfig) {
    CheckNotNull(handle);

    CoSimClient* client = Convert(handle);

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

    return Convert(client->Connect(config));
}

DsVeosCoSim_Result DsVeosCoSim_Disconnect(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    CoSimClient* client = Convert(handle);

    client->Disconnect();

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_GetConnectionState(DsVeosCoSim_Handle handle, DsVeosCoSim_ConnectionState* connectionState) {
    CheckNotNull(handle);
    CheckNotNull(connectionState);

    CoSimClient* client = Convert(handle);

    return Convert(client->GetConnectionState(*Convert(connectionState)));
}

DsVeosCoSim_Result DsVeosCoSim_RunCallbackBasedCoSimulation(DsVeosCoSim_Handle handle, DsVeosCoSim_Callbacks callbacks) {
    CheckNotNull(handle);

    CoSimClient* client = Convert(handle);

    Callbacks newCallbacks{};
    InitializeCallbacks(newCallbacks, callbacks);

    return Convert(client->RunCallbackBasedCoSimulation(newCallbacks));
}

DsVeosCoSim_Result DsVeosCoSim_StartPollingBasedCoSimulation(DsVeosCoSim_Handle handle, DsVeosCoSim_Callbacks callbacks) {
    CheckNotNull(handle);

    CoSimClient* client = Convert(handle);

    Callbacks newCallbacks{};
    InitializeCallbacks(newCallbacks, callbacks);

    return Convert(client->StartPollingBasedCoSimulation(newCallbacks));
}

DsVeosCoSim_Result DsVeosCoSim_PollCommand(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime* simulationTime, DsVeosCoSim_Command* command) {
    CheckNotNull(handle);
    CheckNotNull(simulationTime);
    CheckNotNull(command);

    CoSimClient* client = Convert(handle);

    return Convert(client->PollCommand(*Convert(simulationTime), *Convert(command)));
}

DsVeosCoSim_Result DsVeosCoSim_FinishCommand(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    CoSimClient* client = Convert(handle);

    return Convert(client->FinishCommand());
}

DsVeosCoSim_Result DsVeosCoSim_SetNextSimulationTime(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime simulationTime) {
    CheckNotNull(handle);

    CoSimClient* client = Convert(handle);

    return Convert(client->SetNextSimulationTime(SimulationTime{simulationTime}));
}

DsVeosCoSim_Result DsVeosCoSim_GetStepSize(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime* stepSize) {
    CheckNotNull(handle);
    CheckNotNull(stepSize);

    CoSimClient* client = Convert(handle);

    return Convert(client->GetStepSize(*Convert(stepSize)));
}

DsVeosCoSim_Result DsVeosCoSim_GetIncomingSignals(DsVeosCoSim_Handle handle, uint32_t* incomingSignalsCount, const DsVeosCoSim_IoSignal** incomingSignals) {
    CheckNotNull(handle);
    CheckNotNull(incomingSignalsCount);
    CheckNotNull(incomingSignals);

    CoSimClient* client = Convert(handle);

    return Convert(client->GetIncomingSignals(*incomingSignalsCount, *Convert(incomingSignals)));
}

DsVeosCoSim_Result DsVeosCoSim_ReadIncomingSignal(DsVeosCoSim_Handle handle, DsVeosCoSim_IoSignalId incomingSignalId, uint32_t* length, void* value) {
    CheckNotNull(handle);
    CheckNotNull(length);
    CheckNotNull(value);

    CoSimClient* client = Convert(handle);

    return Convert(client->Read(Convert(incomingSignalId), *length, value));
}

DsVeosCoSim_Result DsVeosCoSim_GetOutgoingSignals(DsVeosCoSim_Handle handle, uint32_t* outgoingSignalsCount, const DsVeosCoSim_IoSignal** outgoingSignals) {
    CheckNotNull(handle);
    CheckNotNull(outgoingSignalsCount);
    CheckNotNull(outgoingSignals);

    CoSimClient* client = Convert(handle);

    return Convert(client->GetOutgoingSignals(*outgoingSignalsCount, *Convert(outgoingSignals)));
}

DsVeosCoSim_Result DsVeosCoSim_WriteOutgoingSignal(DsVeosCoSim_Handle handle, DsVeosCoSim_IoSignalId outgoingSignalId, uint32_t length, const void* value) {
    CheckNotNull(handle);
    if (length > 0) {
        CheckNotNull(value);
    }

    CoSimClient* client = Convert(handle);

    return Convert(client->Write(Convert(outgoingSignalId), length, value));
}

DsVeosCoSim_Result DsVeosCoSim_GetCanControllers(DsVeosCoSim_Handle handle, uint32_t* canControllersCount, const DsVeosCoSim_CanController** canControllers) {
    CheckNotNull(handle);
    CheckNotNull(canControllersCount);
    CheckNotNull(canControllers);

    CoSimClient* client = Convert(handle);

    return Convert(client->GetCanControllers(*canControllersCount, *Convert(canControllers)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveCanMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    CoSimClient* client = Convert(handle);

    return Convert(client->Receive(*Convert(message)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveCanMessageContainer(DsVeosCoSim_Handle handle, DsVeosCoSim_CanMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    CoSimClient* client = Convert(handle);

    return Convert(client->Receive(*Convert(messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitCanMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    CoSimClient* client = Convert(handle);

    return Convert(client->Transmit(*Convert(message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitCanMessageContainer(DsVeosCoSim_Handle handle, const DsVeosCoSim_CanMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    CoSimClient* client = Convert(handle);

    return Convert(client->Transmit(*Convert(messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_GetEthControllers(DsVeosCoSim_Handle handle, uint32_t* ethControllersCount, const DsVeosCoSim_EthController** ethControllers) {
    CheckNotNull(handle);
    CheckNotNull(ethControllersCount);
    CheckNotNull(ethControllers);

    CoSimClient* client = Convert(handle);

    return Convert(client->GetEthControllers(*ethControllersCount, *Convert(ethControllers)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveEthMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    CoSimClient* client = Convert(handle);

    return Convert(client->Receive(*Convert(message)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveEthMessageContainer(DsVeosCoSim_Handle handle, DsVeosCoSim_EthMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    CoSimClient* client = Convert(handle);

    return Convert(client->Receive(*Convert(messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitEthMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    CoSimClient* client = Convert(handle);

    return Convert(client->Transmit(*Convert(message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitEthMessageContainer(DsVeosCoSim_Handle handle, const DsVeosCoSim_EthMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    CoSimClient* client = Convert(handle);

    return Convert(client->Transmit(*Convert(messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_GetLinControllers(DsVeosCoSim_Handle handle, uint32_t* linControllersCount, const DsVeosCoSim_LinController** linControllers) {
    CheckNotNull(handle);
    CheckNotNull(linControllersCount);
    CheckNotNull(linControllers);

    CoSimClient* client = Convert(handle);

    return Convert(client->GetLinControllers(*linControllersCount, *Convert(linControllers)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveLinMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    CoSimClient* client = Convert(handle);

    return Convert(client->Receive(*Convert(message)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveLinMessageContainer(DsVeosCoSim_Handle handle, DsVeosCoSim_LinMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    CoSimClient* client = Convert(handle);

    return Convert(client->Receive(*Convert(messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitLinMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    CoSimClient* client = Convert(handle);

    return Convert(client->Transmit(*Convert(message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitLinMessageContainer(DsVeosCoSim_Handle handle, const DsVeosCoSim_LinMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    CoSimClient* client = Convert(handle);

    return Convert(client->Transmit(*Convert(messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_GetFrControllers(DsVeosCoSim_Handle handle, uint32_t* frControllersCount, const DsVeosCoSim_FrController** frControllers) {
    CheckNotNull(handle);
    CheckNotNull(frControllersCount);
    CheckNotNull(frControllers);

    CoSimClient* client = Convert(handle);

    return Convert(client->GetFrControllers(*frControllersCount, *Convert(frControllers)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveFrMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_FrMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    CoSimClient* client = Convert(handle);

    return Convert(client->Receive(*Convert(message)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveFrMessageContainer(DsVeosCoSim_Handle handle, DsVeosCoSim_FrMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    CoSimClient* client = Convert(handle);

    return Convert(client->Receive(*Convert(messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitFrMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_FrMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    CoSimClient* client = Convert(handle);

    return Convert(client->Transmit(*Convert(message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitFrMessageContainer(DsVeosCoSim_Handle handle, const DsVeosCoSim_FrMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    CoSimClient* client = Convert(handle);

    return Convert(client->Transmit(*Convert(messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_StartSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    CoSimClient* client = Convert(handle);

    return Convert(client->Start());
}

DsVeosCoSim_Result DsVeosCoSim_StopSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    CoSimClient* client = Convert(handle);

    return Convert(client->Stop());
}

DsVeosCoSim_Result DsVeosCoSim_PauseSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    CoSimClient* client = Convert(handle);

    return Convert(client->Pause());
}

DsVeosCoSim_Result DsVeosCoSim_ContinueSimulation(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    CoSimClient* client = Convert(handle);

    return Convert(client->Continue());
}

DsVeosCoSim_Result DsVeosCoSim_TerminateSimulation(DsVeosCoSim_Handle handle, DsVeosCoSim_TerminateReason terminateReason) {
    CheckNotNull(handle);

    CoSimClient* client = Convert(handle);

    return Convert(client->Terminate(Convert(terminateReason)));
}

DsVeosCoSim_Result DsVeosCoSim_GetCurrentSimulationTime(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime* simulationTime) {
    CheckNotNull(handle);
    CheckNotNull(simulationTime);

    CoSimClient* client = Convert(handle);

    return Convert(client->GetCurrentSimulationTime(*Convert(simulationTime)));
}

DsVeosCoSim_Result DsVeosCoSim_GetSimulationState(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationState* simulationState) {
    CheckNotNull(handle);
    CheckNotNull(simulationState);

    CoSimClient* client = Convert(handle);

    return Convert(client->GetSimulationState(*Convert(simulationState)));
}

DsVeosCoSim_Result DsVeosCoSim_GetRoundTripTime(DsVeosCoSim_Handle handle, int64_t* roundTripTimeInNanoseconds) {
    CheckNotNull(handle);
    CheckNotNull(roundTripTimeInNanoseconds);

    CoSimClient* client = Convert(handle);

    SimulationTime roundTripTime{};
    DsVeosCoSim_Result result = Convert(client->GetRoundTripTime(roundTripTime));
    *roundTripTimeInNanoseconds = roundTripTime.count();
    return result;
}

const char* DsVeosCoSim_ResultToString(DsVeosCoSim_Result result) {
    return format_as(Convert(result)).data();
}

const char* DsVeosCoSim_CommandToString(DsVeosCoSim_Command command) {
    return format_as(Convert(command)).data();
}

const char* DsVeosCoSim_SeverityToString(DsVeosCoSim_Severity severity) {
    return format_as(Convert(severity)).data();
}

const char* DsVeosCoSim_TerminateReasonToString(DsVeosCoSim_TerminateReason terminateReason) {
    return format_as(Convert(terminateReason)).data();
}

const char* DsVeosCoSim_ConnectionStateToString(DsVeosCoSim_ConnectionState connectionState) {
    return format_as(Convert(connectionState)).data();
}

const char* DsVeosCoSim_SimulationStateToString(DsVeosCoSim_SimulationState simulationState) {
    return format_as(Convert(simulationState)).data();
}

const char* DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType dataType) {
    return format_as(Convert(dataType)).data();
}

const char* DsVeosCoSim_SizeKindToString(DsVeosCoSim_SizeKind sizeKind) {
    return format_as(Convert(sizeKind)).data();
}

const char* DsVeosCoSim_LinControllerTypeToString(DsVeosCoSim_LinControllerType linControllerType) {
    return format_as(Convert(linControllerType)).data();
}

size_t DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType dataType) {
    return GetDataTypeSize(Convert(dataType));
}

DsVeosCoSim_Result DsVeosCoSim_WriteCanMessageContainerToMessage(const DsVeosCoSim_CanMessageContainer* messageContainer, DsVeosCoSim_CanMessage* message) {
    CheckNotNull(messageContainer);
    CheckNotNull(message);

    Convert(messageContainer)->WriteTo(*Convert(message));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteCanMessageToMessageContainer(const DsVeosCoSim_CanMessage* message, DsVeosCoSim_CanMessageContainer* messageContainer) {
    CheckNotNull(message);
    CheckNotNull(messageContainer);

    Convert(message)->WriteTo(*Convert(messageContainer));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteEthMessageContainerToMessage(const DsVeosCoSim_EthMessageContainer* messageContainer, DsVeosCoSim_EthMessage* message) {
    CheckNotNull(messageContainer);
    CheckNotNull(message);

    Convert(messageContainer)->WriteTo(*Convert(message));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteEthMessageToMessageContainer(const DsVeosCoSim_EthMessage* message, DsVeosCoSim_EthMessageContainer* messageContainer) {
    CheckNotNull(message);
    CheckNotNull(messageContainer);

    Convert(message)->WriteTo(*Convert(messageContainer));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteLinMessageContainerToMessage(const DsVeosCoSim_LinMessageContainer* messageContainer, DsVeosCoSim_LinMessage* message) {
    CheckNotNull(messageContainer);
    CheckNotNull(message);

    Convert(messageContainer)->WriteTo(*Convert(message));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteLinMessageToMessageContainer(const DsVeosCoSim_LinMessage* message, DsVeosCoSim_LinMessageContainer* messageContainer) {
    CheckNotNull(message);
    CheckNotNull(messageContainer);

    Convert(message)->WriteTo(*Convert(messageContainer));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteFrMessageContainerToMessage(const DsVeosCoSim_FrMessageContainer* messageContainer, DsVeosCoSim_FrMessage* message) {
    CheckNotNull(messageContainer);
    CheckNotNull(message);

    Convert(messageContainer)->WriteTo(*Convert(message));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteFrMessageToMessageContainer(const DsVeosCoSim_FrMessage* message, DsVeosCoSim_FrMessageContainer* messageContainer) {
    CheckNotNull(message);
    CheckNotNull(messageContainer);

    Convert(message)->WriteTo(*Convert(messageContainer));

    return DsVeosCoSim_Result_Ok;
}

std::string DsVeosCoSim_SimulationTimeToString(DsVeosCoSim_SimulationTime simulationTime) {
    return SimulationTimeToString(SimulationTime(simulationTime));
}

std::string DsVeosCoSim_IoSignalToString(const DsVeosCoSim_IoSignal* ioSignal) {
    return format_as(*Convert(ioSignal));
}

std::string DsVeosCoSim_CanControllerToString(const DsVeosCoSim_CanController* controller) {
    return format_as(*Convert(controller));
}

std::string DsVeosCoSim_EthControllerToString(const DsVeosCoSim_EthController* controller) {
    return format_as(*Convert(controller));
}

std::string DsVeosCoSim_LinControllerToString(const DsVeosCoSim_LinController* controller) {
    return format_as(*Convert(controller));
}

std::string DsVeosCoSim_FrControllerToString(const DsVeosCoSim_FrController* controller) {
    return format_as(*Convert(controller));
}

std::string DsVeosCoSim_ValueToString(DsVeosCoSim_DataType dataType, uint32_t length, const void* value) {
    return ValueToString(Convert(dataType), length, value);
}

std::string DsVeosCoSim_DataToString(const uint8_t* data, size_t dataLength, char separator) {
    return DataToString(data, dataLength, separator);
}

std::string DsVeosCoSim_IoDataToString(const DsVeosCoSim_IoSignal* ioSignal, uint32_t length, const void* value) {
    return IoDataToString(*Convert(ioSignal), length, value);
}

std::string DsVeosCoSim_CanMessageToString(const DsVeosCoSim_CanMessage* message) {
    return format_as(*Convert(message));
}

std::string DsVeosCoSim_EthMessageToString(const DsVeosCoSim_EthMessage* message) {
    return format_as(*Convert(message));
}

std::string DsVeosCoSim_LinMessageToString(const DsVeosCoSim_LinMessage* message) {
    return format_as(*Convert(message));
}

std::string DsVeosCoSim_FrMessageToString(const DsVeosCoSim_FrMessage* message) {
    return format_as(*Convert(message));
}

std::string DsVeosCoSim_CanMessageContainerToString(const DsVeosCoSim_CanMessageContainer* messageContainer) {
    return format_as(*Convert(messageContainer));
}

std::string DsVeosCoSim_EthMessageContainerToString(const DsVeosCoSim_EthMessageContainer* messageContainer) {
    return format_as(*Convert(messageContainer));
}

std::string DsVeosCoSim_LinMessageContainerToString(const DsVeosCoSim_LinMessageContainer* messageContainer) {
    return format_as(*Convert(messageContainer));
}

std::string DsVeosCoSim_FrMessageContainerToString(const DsVeosCoSim_FrMessageContainer* messageContainer) {
    return format_as(*Convert(messageContainer));
}

std::string DsVeosCoSim_CanMessageFlagsToString(DsVeosCoSim_CanMessageFlags flags) {
    return format_as(ConvertCanMessageFlags(flags));
}

std::string DsVeosCoSim_EthMessageFlagsToString(DsVeosCoSim_EthMessageFlags flags) {
    return format_as(ConvertEthMessageFlags(flags));
}

std::string DsVeosCoSim_LinMessageFlagsToString(DsVeosCoSim_LinMessageFlags flags) {
    return format_as(ConvertLinMessageFlags(flags));
}

std::string DsVeosCoSim_FrMessageFlagsToString(DsVeosCoSim_FrMessageFlags flags) {
    return format_as(ConvertFrMessageFlags(flags));
}

static_assert(CanMessageMaxLength == DSVEOSCOSIM_CAN_MESSAGE_MAX_LENGTH);
static_assert(EthMessageMaxLength == DSVEOSCOSIM_ETH_MESSAGE_MAX_LENGTH);
static_assert(LinMessageMaxLength == DSVEOSCOSIM_LIN_MESSAGE_MAX_LENGTH);
static_assert(FrMessageMaxLength == DSVEOSCOSIM_FLEXRAY_MESSAGE_MAX_LENGTH);
static_assert(EthAddressLength == DSVEOSCOSIM_ETH_ADDRESS_LENGTH);

static_assert(sizeof(void*) == sizeof(DsVeosCoSim_Handle));

static_assert(sizeof(SimulationTime) == sizeof(DsVeosCoSim_SimulationTime));

static_assert(sizeof(CoSimType) == sizeof(uint32_t));

static_assert(sizeof(ConnectionKind) == sizeof(uint32_t));

static_assert(sizeof(Result) == sizeof(DsVeosCoSim_Result));
static_assert(CreateOk() == Convert(DsVeosCoSim_Result_Ok));
static_assert(CreateError() == Convert(DsVeosCoSim_Result_Error));
static_assert(CreateEmpty() == Convert(DsVeosCoSim_Result_Empty));
static_assert(CreateFull() == Convert(DsVeosCoSim_Result_Full));
static_assert(CreateInvalidArgument() == Convert(DsVeosCoSim_Result_InvalidArgument));
static_assert(CreateNotConnected() == Convert(DsVeosCoSim_Result_Disconnected));
static_assert(CreateTimeout() == Convert(DsVeosCoSim_Result_Timeout));

static_assert(sizeof(Command) == sizeof(DsVeosCoSim_Command));
static_assert(Command::None == Convert(DsVeosCoSim_Command_None));
static_assert(Command::Step == Convert(DsVeosCoSim_Command_Step));
static_assert(Command::Start == Convert(DsVeosCoSim_Command_Start));
static_assert(Command::Stop == Convert(DsVeosCoSim_Command_Stop));
static_assert(Command::Terminate == Convert(DsVeosCoSim_Command_Terminate));
static_assert(Command::Pause == Convert(DsVeosCoSim_Command_Pause));
static_assert(Command::Continue == Convert(DsVeosCoSim_Command_Continue));
static_assert(Command::TerminateFinished == Convert(DsVeosCoSim_Command_TerminateFinished));
static_assert(Command::Ping == Convert(DsVeosCoSim_Command_Ping));

static_assert(sizeof(Severity) == sizeof(DsVeosCoSim_Severity));
static_assert(Severity::Error == Convert(DsVeosCoSim_Severity_Error));
static_assert(Severity::Warning == Convert(DsVeosCoSim_Severity_Warning));
static_assert(Severity::Info == Convert(DsVeosCoSim_Severity_Info));
static_assert(Severity::Trace == Convert(DsVeosCoSim_Severity_Trace));

static_assert(sizeof(TerminateReason) == sizeof(DsVeosCoSim_TerminateReason));
static_assert(TerminateReason::Finished == Convert(DsVeosCoSim_TerminateReason_Finished));
static_assert(TerminateReason::Error == Convert(DsVeosCoSim_TerminateReason_Error));

static_assert(sizeof(ConnectionState) == sizeof(DsVeosCoSim_ConnectionState));
static_assert(ConnectionState::Disconnected == Convert(DsVeosCoSim_ConnectionState_Disconnected));
static_assert(ConnectionState::Connected == Convert(DsVeosCoSim_ConnectionState_Connected));

static_assert(sizeof(SimulationState) == sizeof(DsVeosCoSim_SimulationState));
static_assert(SimulationState::Unloaded == Convert(DsVeosCoSim_SimulationState_Unloaded));
static_assert(SimulationState::Stopped == Convert(DsVeosCoSim_SimulationState_Stopped));
static_assert(SimulationState::Running == Convert(DsVeosCoSim_SimulationState_Running));
static_assert(SimulationState::Paused == Convert(DsVeosCoSim_SimulationState_Paused));
static_assert(SimulationState::Terminated == Convert(DsVeosCoSim_SimulationState_Terminated));

static_assert(sizeof(IoSignalId) == sizeof(DsVeosCoSim_IoSignalId));

static_assert(sizeof(DataType) == sizeof(DsVeosCoSim_DataType));
static_assert(DataType::Bool == Convert(DsVeosCoSim_DataType_Bool));
static_assert(DataType::Int8 == Convert(DsVeosCoSim_DataType_Int8));
static_assert(DataType::Int16 == Convert(DsVeosCoSim_DataType_Int16));
static_assert(DataType::Int32 == Convert(DsVeosCoSim_DataType_Int32));
static_assert(DataType::Int64 == Convert(DsVeosCoSim_DataType_Int64));
static_assert(DataType::UInt8 == Convert(DsVeosCoSim_DataType_UInt8));
static_assert(DataType::UInt16 == Convert(DsVeosCoSim_DataType_UInt16));
static_assert(DataType::UInt32 == Convert(DsVeosCoSim_DataType_UInt32));
static_assert(DataType::UInt64 == Convert(DsVeosCoSim_DataType_UInt64));
static_assert(DataType::Float32 == Convert(DsVeosCoSim_DataType_Float32));
static_assert(DataType::Float64 == Convert(DsVeosCoSim_DataType_Float64));

static_assert(sizeof(SizeKind) == sizeof(DsVeosCoSim_SizeKind));
static_assert(SizeKind::Fixed == Convert(DsVeosCoSim_SizeKind_Fixed));
static_assert(SizeKind::Variable == Convert(DsVeosCoSim_SizeKind_Variable));

static_assert(sizeof(BusControllerId) == sizeof(DsVeosCoSim_BusControllerId));

static_assert(sizeof(BusMessageId) == sizeof(uint32_t));

static_assert(sizeof(LinControllerType) == sizeof(DsVeosCoSim_LinControllerType));
static_assert(LinControllerType::Responder == Convert(DsVeosCoSim_LinControllerType_Responder));
static_assert(LinControllerType::Commander == Convert(DsVeosCoSim_LinControllerType_Commander));

static_assert(sizeof(CanMessageFlags) == sizeof(DsVeosCoSim_CanMessageFlags));
static_assert(CanMessageFlags::Loopback == ConvertCanMessageFlags(DsVeosCoSim_CanMessageFlags_Loopback));
static_assert(CanMessageFlags::Error == ConvertCanMessageFlags(DsVeosCoSim_CanMessageFlags_Error));
static_assert(CanMessageFlags::Drop == ConvertCanMessageFlags(DsVeosCoSim_CanMessageFlags_Drop));
static_assert(CanMessageFlags::ExtendedId == ConvertCanMessageFlags(DsVeosCoSim_CanMessageFlags_ExtendedId));
static_assert(CanMessageFlags::BitRateSwitch == ConvertCanMessageFlags(DsVeosCoSim_CanMessageFlags_BitRateSwitch));
static_assert(CanMessageFlags::FlexibleDataRateFormat == ConvertCanMessageFlags(DsVeosCoSim_CanMessageFlags_FlexibleDataRateFormat));

static_assert(sizeof(EthMessageFlags) == sizeof(DsVeosCoSim_EthMessageFlags));
static_assert(EthMessageFlags::Loopback == ConvertEthMessageFlags(DsVeosCoSim_EthMessageFlags_Loopback));
static_assert(EthMessageFlags::Error == ConvertEthMessageFlags(DsVeosCoSim_EthMessageFlags_Error));
static_assert(EthMessageFlags::Drop == ConvertEthMessageFlags(DsVeosCoSim_EthMessageFlags_Drop));

static_assert(sizeof(LinMessageFlags) == sizeof(DsVeosCoSim_LinMessageFlags));
static_assert(LinMessageFlags::Loopback == ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags_Loopback));
static_assert(LinMessageFlags::Error == ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags_Error));
static_assert(LinMessageFlags::Drop == ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags_Drop));
static_assert(LinMessageFlags::Header == ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags_Header));
static_assert(LinMessageFlags::Response == ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags_Response));
static_assert(LinMessageFlags::WakeEvent == ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags_WakeEvent));
static_assert(LinMessageFlags::SleepEvent == ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags_SleepEvent));
static_assert(LinMessageFlags::EnhancedChecksum == ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags_EnhancedChecksum));
static_assert(LinMessageFlags::TransferOnce == ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags_TransferOnce));
static_assert(LinMessageFlags::ParityFailure == ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags_ParityFailure));
static_assert(LinMessageFlags::Collision == ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags_Collision));
static_assert(LinMessageFlags::NoResponse == ConvertLinMessageFlags(DsVeosCoSim_LinMessageFlags_NoResponse));

static_assert(sizeof(FrMessageFlags) == sizeof(DsVeosCoSim_FrMessageFlags));
static_assert(FrMessageFlags::Startup == ConvertFrMessageFlags(DsVeosCoSim_FrMessageFlags_Startup));
static_assert(FrMessageFlags::SyncFrame == ConvertFrMessageFlags(DsVeosCoSim_FrMessageFlags_SyncFrame));
static_assert(FrMessageFlags::NullFrame == ConvertFrMessageFlags(DsVeosCoSim_FrMessageFlags_NullFrame));
static_assert(FrMessageFlags::PayloadPreamble == ConvertFrMessageFlags(DsVeosCoSim_FrMessageFlags_PayloadPreamble));
static_assert(FrMessageFlags::Loopback == ConvertFrMessageFlags(DsVeosCoSim_FrMessageFlags_Loopback));
static_assert(FrMessageFlags::TransferOnce == ConvertFrMessageFlags(DsVeosCoSim_FrMessageFlags_TransferOnce));
static_assert(FrMessageFlags::ChannelA == ConvertFrMessageFlags(DsVeosCoSim_FrMessageFlags_ChannelA));
static_assert(FrMessageFlags::ChannelB == ConvertFrMessageFlags(DsVeosCoSim_FrMessageFlags_ChannelB));
static_assert(FrMessageFlags::Error == ConvertFrMessageFlags(DsVeosCoSim_FrMessageFlags_Error));
static_assert(FrMessageFlags::Drop == ConvertFrMessageFlags(DsVeosCoSim_FrMessageFlags_Drop));

static_assert(sizeof(FrameKind) == sizeof(uint32_t));

static_assert(sizeof(IoSignal) == sizeof(DsVeosCoSim_IoSignal));
static_assert(offsetof(IoSignal, id) == offsetof(DsVeosCoSim_IoSignal, id));
static_assert(offsetof(IoSignal, length) == offsetof(DsVeosCoSim_IoSignal, length));
static_assert(offsetof(IoSignal, dataType) == offsetof(DsVeosCoSim_IoSignal, dataType));
static_assert(offsetof(IoSignal, sizeKind) == offsetof(DsVeosCoSim_IoSignal, sizeKind));
static_assert(offsetof(IoSignal, name) == offsetof(DsVeosCoSim_IoSignal, name));

static_assert(sizeof(CanController) == sizeof(DsVeosCoSim_CanController));
static_assert(offsetof(CanController, id) == offsetof(DsVeosCoSim_CanController, id));
static_assert(offsetof(CanController, queueSize) == offsetof(DsVeosCoSim_CanController, queueSize));
static_assert(offsetof(CanController, bitsPerSecond) == offsetof(DsVeosCoSim_CanController, bitsPerSecond));
static_assert(offsetof(CanController, flexibleDataRateBitsPerSecond) == offsetof(DsVeosCoSim_CanController, flexibleDataRateBitsPerSecond));
static_assert(offsetof(CanController, name) == offsetof(DsVeosCoSim_CanController, name));
static_assert(offsetof(CanController, channelName) == offsetof(DsVeosCoSim_CanController, channelName));
static_assert(offsetof(CanController, clusterName) == offsetof(DsVeosCoSim_CanController, clusterName));

static_assert(sizeof(CanMessage) == sizeof(DsVeosCoSim_CanMessage));
static_assert(offsetof(CanMessage, timestamp) == offsetof(DsVeosCoSim_CanMessage, timestamp));
static_assert(offsetof(CanMessage, controllerId) == offsetof(DsVeosCoSim_CanMessage, controllerId));
static_assert(offsetof(CanMessage, id) == offsetof(DsVeosCoSim_CanMessage, id));
static_assert(offsetof(CanMessage, flags) == offsetof(DsVeosCoSim_CanMessage, flags));
static_assert(offsetof(CanMessage, length) == offsetof(DsVeosCoSim_CanMessage, length));
static_assert(offsetof(CanMessage, data) == offsetof(DsVeosCoSim_CanMessage, data));

static_assert(sizeof(CanMessageContainer) == sizeof(DsVeosCoSim_CanMessageContainer));
static_assert(offsetof(CanMessageContainer, timestamp) == offsetof(DsVeosCoSim_CanMessageContainer, timestamp));
static_assert(offsetof(CanMessageContainer, controllerId) == offsetof(DsVeosCoSim_CanMessageContainer, controllerId));
static_assert(offsetof(CanMessageContainer, reserved) == offsetof(DsVeosCoSim_CanMessageContainer, reserved));
static_assert(offsetof(CanMessageContainer, id) == offsetof(DsVeosCoSim_CanMessageContainer, id));
static_assert(offsetof(CanMessageContainer, flags) == offsetof(DsVeosCoSim_CanMessageContainer, flags));
static_assert(offsetof(CanMessageContainer, length) == offsetof(DsVeosCoSim_CanMessageContainer, length));
static_assert(offsetof(CanMessageContainer, data) == offsetof(DsVeosCoSim_CanMessageContainer, data));

static_assert(sizeof(EthController) == sizeof(DsVeosCoSim_EthController));
static_assert(offsetof(EthController, id) == offsetof(DsVeosCoSim_EthController, id));
static_assert(offsetof(EthController, queueSize) == offsetof(DsVeosCoSim_EthController, queueSize));
static_assert(offsetof(EthController, bitsPerSecond) == offsetof(DsVeosCoSim_EthController, bitsPerSecond));
static_assert(offsetof(EthController, macAddress) == offsetof(DsVeosCoSim_EthController, macAddress));
static_assert(offsetof(EthController, name) == offsetof(DsVeosCoSim_EthController, name));
static_assert(offsetof(EthController, channelName) == offsetof(DsVeosCoSim_EthController, channelName));
static_assert(offsetof(EthController, clusterName) == offsetof(DsVeosCoSim_EthController, clusterName));

static_assert(sizeof(EthMessage) == sizeof(DsVeosCoSim_EthMessage));
static_assert(offsetof(EthMessage, timestamp) == offsetof(DsVeosCoSim_EthMessage, timestamp));
static_assert(offsetof(EthMessage, controllerId) == offsetof(DsVeosCoSim_EthMessage, controllerId));
static_assert(offsetof(EthMessage, reserved) == offsetof(DsVeosCoSim_EthMessage, reserved));
static_assert(offsetof(EthMessage, flags) == offsetof(DsVeosCoSim_EthMessage, flags));
static_assert(offsetof(EthMessage, length) == offsetof(DsVeosCoSim_EthMessage, length));
static_assert(offsetof(EthMessage, data) == offsetof(DsVeosCoSim_EthMessage, data));

static_assert(sizeof(EthMessageContainer) == sizeof(DsVeosCoSim_EthMessageContainer));
static_assert(offsetof(EthMessageContainer, timestamp) == offsetof(DsVeosCoSim_EthMessageContainer, timestamp));
static_assert(offsetof(EthMessageContainer, controllerId) == offsetof(DsVeosCoSim_EthMessageContainer, controllerId));
static_assert(offsetof(EthMessageContainer, reserved) == offsetof(DsVeosCoSim_EthMessageContainer, reserved));
static_assert(offsetof(EthMessageContainer, flags) == offsetof(DsVeosCoSim_EthMessageContainer, flags));
static_assert(offsetof(EthMessageContainer, length) == offsetof(DsVeosCoSim_EthMessageContainer, length));
static_assert(offsetof(EthMessageContainer, data) == offsetof(DsVeosCoSim_EthMessageContainer, data));

static_assert(sizeof(LinController) == sizeof(DsVeosCoSim_LinController));
static_assert(offsetof(LinController, id) == offsetof(DsVeosCoSim_LinController, id));
static_assert(offsetof(LinController, queueSize) == offsetof(DsVeosCoSim_LinController, queueSize));
static_assert(offsetof(LinController, bitsPerSecond) == offsetof(DsVeosCoSim_LinController, bitsPerSecond));
static_assert(offsetof(LinController, type) == offsetof(DsVeosCoSim_LinController, type));
static_assert(offsetof(LinController, name) == offsetof(DsVeosCoSim_LinController, name));
static_assert(offsetof(LinController, channelName) == offsetof(DsVeosCoSim_LinController, channelName));
static_assert(offsetof(LinController, clusterName) == offsetof(DsVeosCoSim_LinController, clusterName));

static_assert(sizeof(LinMessage) == sizeof(DsVeosCoSim_LinMessage));
static_assert(offsetof(LinMessage, timestamp) == offsetof(DsVeosCoSim_LinMessage, timestamp));
static_assert(offsetof(LinMessage, controllerId) == offsetof(DsVeosCoSim_LinMessage, controllerId));
static_assert(offsetof(LinMessage, id) == offsetof(DsVeosCoSim_LinMessage, id));
static_assert(offsetof(LinMessage, flags) == offsetof(DsVeosCoSim_LinMessage, flags));
static_assert(offsetof(LinMessage, length) == offsetof(DsVeosCoSim_LinMessage, length));
static_assert(offsetof(LinMessage, data) == offsetof(DsVeosCoSim_LinMessage, data));

static_assert(sizeof(LinMessageContainer) == sizeof(DsVeosCoSim_LinMessageContainer));
static_assert(offsetof(LinMessageContainer, timestamp) == offsetof(DsVeosCoSim_LinMessageContainer, timestamp));
static_assert(offsetof(LinMessageContainer, controllerId) == offsetof(DsVeosCoSim_LinMessageContainer, controllerId));
static_assert(offsetof(LinMessageContainer, reserved) == offsetof(DsVeosCoSim_LinMessageContainer, reserved));
static_assert(offsetof(LinMessageContainer, id) == offsetof(DsVeosCoSim_LinMessageContainer, id));
static_assert(offsetof(LinMessageContainer, flags) == offsetof(DsVeosCoSim_LinMessageContainer, flags));
static_assert(offsetof(LinMessageContainer, length) == offsetof(DsVeosCoSim_LinMessageContainer, length));
static_assert(offsetof(LinMessageContainer, data) == offsetof(DsVeosCoSim_LinMessageContainer, data));

static_assert(sizeof(FrController) == sizeof(DsVeosCoSim_FrController));
static_assert(offsetof(FrController, id) == offsetof(DsVeosCoSim_FrController, id));
static_assert(offsetof(FrController, queueSize) == offsetof(DsVeosCoSim_FrController, queueSize));
static_assert(offsetof(FrController, bitsPerSecond) == offsetof(DsVeosCoSim_FrController, bitsPerSecond));
static_assert(offsetof(FrController, name) == offsetof(DsVeosCoSim_FrController, name));
static_assert(offsetof(FrController, channelName) == offsetof(DsVeosCoSim_FrController, channelName));
static_assert(offsetof(FrController, clusterName) == offsetof(DsVeosCoSim_FrController, clusterName));

static_assert(sizeof(FrMessage) == sizeof(DsVeosCoSim_FrMessage));
static_assert(offsetof(FrMessage, timestamp) == offsetof(DsVeosCoSim_FrMessage, timestamp));
static_assert(offsetof(FrMessage, controllerId) == offsetof(DsVeosCoSim_FrMessage, controllerId));
static_assert(offsetof(FrMessage, id) == offsetof(DsVeosCoSim_FrMessage, id));
static_assert(offsetof(FrMessage, flags) == offsetof(DsVeosCoSim_FrMessage, flags));
static_assert(offsetof(FrMessage, length) == offsetof(DsVeosCoSim_FrMessage, length));
static_assert(offsetof(FrMessage, data) == offsetof(DsVeosCoSim_FrMessage, data));

static_assert(sizeof(FrMessageContainer) == sizeof(DsVeosCoSim_FrMessageContainer));
static_assert(offsetof(FrMessageContainer, timestamp) == offsetof(DsVeosCoSim_FrMessageContainer, timestamp));
static_assert(offsetof(FrMessageContainer, controllerId) == offsetof(DsVeosCoSim_FrMessageContainer, controllerId));
static_assert(offsetof(FrMessageContainer, reserved) == offsetof(DsVeosCoSim_FrMessageContainer, reserved));
static_assert(offsetof(FrMessageContainer, id) == offsetof(DsVeosCoSim_FrMessageContainer, id));
static_assert(offsetof(FrMessageContainer, flags) == offsetof(DsVeosCoSim_FrMessageContainer, flags));
static_assert(offsetof(FrMessageContainer, length) == offsetof(DsVeosCoSim_FrMessageContainer, length));
static_assert(offsetof(FrMessageContainer, data) == offsetof(DsVeosCoSim_FrMessageContainer, data));
