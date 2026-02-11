// Copyright dSPACE SE & Co. KG. All rights reserved.

#include "DsVeosCoSim/DsVeosCoSim.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "DsVeosCoSim/CoSimClient.h"
#include "DsVeosCoSim/CoSimTypes.h"

using namespace DsVeosCoSim;

namespace {

#define CheckNotNull(arg)                                                       \
    do {                                                                        \
        if (!(arg)) {                                                           \
            Logger::Instance().LogError("Argument " #arg " must not be null."); \
            return DsVeosCoSim_Result_InvalidArgument;                          \
        }                                                                       \
    } while (0)

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
        newCallbacks.canMessageReceivedCallback = [=](SimulationTime simulationTime, const CanController& canController, const CanMessage& message) {
            canMessageReceivedCallback(simulationTime.count(),
                                       reinterpret_cast<const DsVeosCoSim_CanController*>(&canController),
                                       reinterpret_cast<const DsVeosCoSim_CanMessage*>(&message),
                                       userData);
        };
    }

    if (canMessageContainerReceivedCallback) {
        newCallbacks.canMessageContainerReceivedCallback =
            [=](SimulationTime simulationTime, const CanController& canController, const CanMessageContainer& messageContainer) {
                canMessageContainerReceivedCallback(simulationTime.count(),
                                                    reinterpret_cast<const DsVeosCoSim_CanController*>(&canController),
                                                    reinterpret_cast<const DsVeosCoSim_CanMessageContainer*>(&messageContainer),
                                                    userData);
            };
    }

    if (ethMessageReceivedCallback) {
        newCallbacks.ethMessageReceivedCallback = [=](SimulationTime simulationTime, const EthController& ethController, const EthMessage& message) {
            ethMessageReceivedCallback(simulationTime.count(),
                                       reinterpret_cast<const DsVeosCoSim_EthController*>(&ethController),
                                       reinterpret_cast<const DsVeosCoSim_EthMessage*>(&message),
                                       userData);
        };
    }

    if (ethMessageContainerReceivedCallback) {
        newCallbacks.ethMessageContainerReceivedCallback =
            [=](SimulationTime simulationTime, const EthController& ethController, const EthMessageContainer& messageContainer) {
                ethMessageContainerReceivedCallback(simulationTime.count(),
                                                    reinterpret_cast<const DsVeosCoSim_EthController*>(&ethController),
                                                    reinterpret_cast<const DsVeosCoSim_EthMessageContainer*>(&messageContainer),
                                                    userData);
            };
    }

    if (linMessageReceivedCallback) {
        newCallbacks.linMessageReceivedCallback = [=](SimulationTime simulationTime, const LinController& linController, const LinMessage& message) {
            linMessageReceivedCallback(simulationTime.count(),
                                       reinterpret_cast<const DsVeosCoSim_LinController*>(&linController),
                                       reinterpret_cast<const DsVeosCoSim_LinMessage*>(&message),
                                       userData);
        };
    }

    if (linMessageContainerReceivedCallback) {
        newCallbacks.linMessageContainerReceivedCallback =
            [=](SimulationTime simulationTime, const LinController& linController, const LinMessageContainer& messageContainer) {
                linMessageContainerReceivedCallback(simulationTime.count(),
                                                    reinterpret_cast<const DsVeosCoSim_LinController*>(&linController),
                                                    reinterpret_cast<const DsVeosCoSim_LinMessageContainer*>(&messageContainer),
                                                    userData);
            };
    }

    if (frMessageReceivedCallback) {
        newCallbacks.frMessageReceivedCallback = [=](SimulationTime simulationTime, const FrController& frController, const FrMessage& message) {
            frMessageReceivedCallback(simulationTime.count(),
                                      reinterpret_cast<const DsVeosCoSim_FrController*>(&frController),
                                      reinterpret_cast<const DsVeosCoSim_FrMessage*>(&message),
                                      userData);
        };
    }

    if (frMessageContainerReceivedCallback) {
        newCallbacks.frMessageContainerReceivedCallback =
            [=](SimulationTime simulationTime, const FrController& frController, const FrMessageContainer& messageContainer) {
                frMessageContainerReceivedCallback(simulationTime.count(),
                                                   reinterpret_cast<const DsVeosCoSim_FrController*>(&frController),
                                                   reinterpret_cast<const DsVeosCoSim_FrMessageContainer*>(&messageContainer),
                                                   userData);
            };
    }

    if (incomingSignalChangedCallback) {
        newCallbacks.incomingSignalChangedCallback = [=](SimulationTime simulationTime, const IoSignal& ioSignal, uint32_t length, const void* value) {
            incomingSignalChangedCallback(simulationTime.count(), reinterpret_cast<const DsVeosCoSim_IoSignal*>(&ioSignal), length, value, userData);
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
            simulationTerminatedCallback(simulationTime.count(), static_cast<DsVeosCoSim_TerminateReason>(reason), userData);
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
            logCallback(static_cast<DsVeosCoSim_Severity>(severity), message.c_str());
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

DsVeosCoSim_Result DsVeosCoSim_GetConnectionState(DsVeosCoSim_Handle handle, DsVeosCoSim_ConnectionState* connectionState) {
    CheckNotNull(handle);
    CheckNotNull(connectionState);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetConnectionState(*reinterpret_cast<ConnectionState*>(connectionState)));
}

DsVeosCoSim_Result DsVeosCoSim_RunCallbackBasedCoSimulation(DsVeosCoSim_Handle handle, DsVeosCoSim_Callbacks callbacks) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

    Callbacks newCallbacks{};
    InitializeCallbacks(newCallbacks, callbacks);

    return static_cast<DsVeosCoSim_Result>(client->RunCallbackBasedCoSimulation(newCallbacks));
}

DsVeosCoSim_Result DsVeosCoSim_StartPollingBasedCoSimulation(DsVeosCoSim_Handle handle, DsVeosCoSim_Callbacks callbacks) {
    CheckNotNull(handle);

    auto* const client = static_cast<CoSimClient*>(handle);

    Callbacks newCallbacks{};
    InitializeCallbacks(newCallbacks, callbacks);

    return static_cast<DsVeosCoSim_Result>(client->StartPollingBasedCoSimulation(newCallbacks));
}

DsVeosCoSim_Result DsVeosCoSim_PollCommand(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime* simulationTime, DsVeosCoSim_Command* command) {
    CheckNotNull(handle);
    CheckNotNull(simulationTime);
    CheckNotNull(command);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->PollCommand(*reinterpret_cast<SimulationTime*>(simulationTime), *reinterpret_cast<Command*>(command)));
}

DsVeosCoSim_Result DsVeosCoSim_FinishCommand(DsVeosCoSim_Handle handle) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->FinishCommand());
}

DsVeosCoSim_Result DsVeosCoSim_SetNextSimulationTime(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime simulationTime) {
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

DsVeosCoSim_Result DsVeosCoSim_GetIncomingSignals(DsVeosCoSim_Handle handle, uint32_t* incomingSignalsCount, const DsVeosCoSim_IoSignal** incomingSignals) {
    CheckNotNull(handle);
    CheckNotNull(incomingSignalsCount);
    CheckNotNull(incomingSignals);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetIncomingSignals(*incomingSignalsCount, *reinterpret_cast<const IoSignal**>(incomingSignals)));
}

DsVeosCoSim_Result DsVeosCoSim_ReadIncomingSignal(DsVeosCoSim_Handle handle, DsVeosCoSim_IoSignalId incomingSignalId, uint32_t* length, void* value) {
    CheckNotNull(handle);
    CheckNotNull(length);
    CheckNotNull(value);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Read(static_cast<IoSignalId>(incomingSignalId), *length, value));
}

DsVeosCoSim_Result DsVeosCoSim_GetOutgoingSignals(DsVeosCoSim_Handle handle, uint32_t* outgoingSignalsCount, const DsVeosCoSim_IoSignal** outgoingSignals) {
    CheckNotNull(handle);
    CheckNotNull(outgoingSignalsCount);
    CheckNotNull(outgoingSignals);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetOutgoingSignals(*outgoingSignalsCount, *reinterpret_cast<const IoSignal**>(outgoingSignals)));
}

DsVeosCoSim_Result DsVeosCoSim_WriteOutgoingSignal(DsVeosCoSim_Handle handle, DsVeosCoSim_IoSignalId outgoingSignalId, uint32_t length, const void* value) {
    CheckNotNull(handle);
    if (length > 0) {
        CheckNotNull(value);
    }

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Write(static_cast<IoSignalId>(outgoingSignalId), length, value));
}

DsVeosCoSim_Result DsVeosCoSim_GetCanControllers(DsVeosCoSim_Handle handle, uint32_t* canControllersCount, const DsVeosCoSim_CanController** canControllers) {
    CheckNotNull(handle);
    CheckNotNull(canControllersCount);
    CheckNotNull(canControllers);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetCanControllers(*canControllersCount, *reinterpret_cast<const CanController**>(canControllers)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveCanMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(reinterpret_cast<CanMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveCanMessageContainer(DsVeosCoSim_Handle handle, DsVeosCoSim_CanMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(reinterpret_cast<CanMessageContainer&>(*messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitCanMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_CanMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(reinterpret_cast<const CanMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitCanMessageContainer(DsVeosCoSim_Handle handle, const DsVeosCoSim_CanMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(reinterpret_cast<const CanMessageContainer&>(*messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_GetEthControllers(DsVeosCoSim_Handle handle, uint32_t* ethControllersCount, const DsVeosCoSim_EthController** ethControllers) {
    CheckNotNull(handle);
    CheckNotNull(ethControllersCount);
    CheckNotNull(ethControllers);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetEthControllers(*ethControllersCount, *reinterpret_cast<const EthController**>(ethControllers)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveEthMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(reinterpret_cast<EthMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveEthMessageContainer(DsVeosCoSim_Handle handle, DsVeosCoSim_EthMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(reinterpret_cast<EthMessageContainer&>(*messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitEthMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_EthMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(reinterpret_cast<const EthMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitEthMessageContainer(DsVeosCoSim_Handle handle, const DsVeosCoSim_EthMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(reinterpret_cast<const EthMessageContainer&>(*messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_GetLinControllers(DsVeosCoSim_Handle handle, uint32_t* linControllersCount, const DsVeosCoSim_LinController** linControllers) {
    CheckNotNull(handle);
    CheckNotNull(linControllersCount);
    CheckNotNull(linControllers);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetLinControllers(*linControllersCount, *reinterpret_cast<const LinController**>(linControllers)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveLinMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(reinterpret_cast<LinMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveLinMessageContainer(DsVeosCoSim_Handle handle, DsVeosCoSim_LinMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(reinterpret_cast<LinMessageContainer&>(*messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitLinMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_LinMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(reinterpret_cast<const LinMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitLinMessageContainer(DsVeosCoSim_Handle handle, const DsVeosCoSim_LinMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(reinterpret_cast<const LinMessageContainer&>(*messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_GetFrControllers(DsVeosCoSim_Handle handle, uint32_t* frControllersCount, const DsVeosCoSim_FrController** frControllers) {
    CheckNotNull(handle);
    CheckNotNull(frControllersCount);
    CheckNotNull(frControllers);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetFrControllers(*frControllersCount, *reinterpret_cast<const FrController**>(frControllers)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveFrMessage(DsVeosCoSim_Handle handle, DsVeosCoSim_FrMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(reinterpret_cast<FrMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_ReceiveFrMessageContainer(DsVeosCoSim_Handle handle, DsVeosCoSim_FrMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Receive(reinterpret_cast<FrMessageContainer&>(*messageContainer)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitFrMessage(DsVeosCoSim_Handle handle, const DsVeosCoSim_FrMessage* message) {
    CheckNotNull(handle);
    CheckNotNull(message);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(reinterpret_cast<const FrMessage&>(*message)));
}

DsVeosCoSim_Result DsVeosCoSim_TransmitFrMessageContainer(DsVeosCoSim_Handle handle, const DsVeosCoSim_FrMessageContainer* messageContainer) {
    CheckNotNull(handle);
    CheckNotNull(messageContainer);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Transmit(reinterpret_cast<const FrMessageContainer&>(*messageContainer)));
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

DsVeosCoSim_Result DsVeosCoSim_TerminateSimulation(DsVeosCoSim_Handle handle, DsVeosCoSim_TerminateReason terminateReason) {
    CheckNotNull(handle);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->Terminate(static_cast<TerminateReason>(terminateReason)));
}

DsVeosCoSim_Result DsVeosCoSim_GetCurrentSimulationTime(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationTime* simulationTime) {
    CheckNotNull(handle);
    CheckNotNull(simulationTime);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetCurrentSimulationTime(*reinterpret_cast<SimulationTime*>(simulationTime)));
}

DsVeosCoSim_Result DsVeosCoSim_GetSimulationState(DsVeosCoSim_Handle handle, DsVeosCoSim_SimulationState* simulationState) {
    CheckNotNull(handle);
    CheckNotNull(simulationState);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetSimulationState(*reinterpret_cast<SimulationState*>(simulationState)));
}

DsVeosCoSim_Result DsVeosCoSim_GetRoundTripTime(DsVeosCoSim_Handle handle, int64_t* roundTripTimeInNanoseconds) {
    CheckNotNull(handle);
    CheckNotNull(roundTripTimeInNanoseconds);

    auto* client = static_cast<CoSimClient*>(handle);

    return static_cast<DsVeosCoSim_Result>(client->GetRoundTripTime(*reinterpret_cast<std::chrono::nanoseconds*>(roundTripTimeInNanoseconds)));
}

const char* DsVeosCoSim_ResultToString(DsVeosCoSim_Result result) {
    return format_as(static_cast<Result>(result));
}

const char* DsVeosCoSim_CommandToString(DsVeosCoSim_Command command) {
    return format_as(static_cast<Command>(command));
}

const char* DsVeosCoSim_SeverityToString(DsVeosCoSim_Severity severity) {
    return format_as(static_cast<Severity>(severity));
}

const char* DsVeosCoSim_TerminateReasonToString(DsVeosCoSim_TerminateReason terminateReason) {
    return format_as(static_cast<TerminateReason>(terminateReason));
}

const char* DsVeosCoSim_ConnectionStateToString(DsVeosCoSim_ConnectionState connectionState) {
    return format_as(static_cast<ConnectionState>(connectionState));
}

const char* DsVeosCoSim_SimulationStateToString(DsVeosCoSim_SimulationState simulationState) {
    return format_as(static_cast<SimulationState>(simulationState));
}

const char* DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType dataType) {
    return format_as(static_cast<DataType>(dataType));
}

const char* DsVeosCoSim_SizeKindToString(DsVeosCoSim_SizeKind sizeKind) {
    return format_as(static_cast<SizeKind>(sizeKind));
}

const char* DsVeosCoSim_LinControllerTypeToString(DsVeosCoSim_LinControllerType linControllerType) {
    return format_as(static_cast<LinControllerType>(linControllerType));
}

size_t DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType dataType) {
    return GetDataTypeSize(static_cast<DataType>(dataType));
}

DsVeosCoSim_Result DsVeosCoSim_WriteCanMessageContainerToMessage(const DsVeosCoSim_CanMessageContainer* messageContainer, DsVeosCoSim_CanMessage* message) {
    CheckNotNull(messageContainer);
    CheckNotNull(message);

    reinterpret_cast<const CanMessageContainer*>(messageContainer)->WriteTo(*reinterpret_cast<CanMessage*>(message));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteCanMessageToMessageContainer(const DsVeosCoSim_CanMessage* message, DsVeosCoSim_CanMessageContainer* messageContainer) {
    CheckNotNull(message);
    CheckNotNull(messageContainer);

    reinterpret_cast<const CanMessage*>(message)->WriteTo(*reinterpret_cast<CanMessageContainer*>(messageContainer));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteEthMessageContainerToMessage(const DsVeosCoSim_EthMessageContainer* messageContainer, DsVeosCoSim_EthMessage* message) {
    CheckNotNull(messageContainer);
    CheckNotNull(message);

    reinterpret_cast<const EthMessageContainer*>(messageContainer)->WriteTo(*reinterpret_cast<EthMessage*>(message));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteEthMessageToMessageContainer(const DsVeosCoSim_EthMessage* message, DsVeosCoSim_EthMessageContainer* messageContainer) {
    CheckNotNull(message);
    CheckNotNull(messageContainer);

    reinterpret_cast<const EthMessage*>(message)->WriteTo(*reinterpret_cast<EthMessageContainer*>(messageContainer));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteLinMessageContainerToMessage(const DsVeosCoSim_LinMessageContainer* messageContainer, DsVeosCoSim_LinMessage* message) {
    CheckNotNull(messageContainer);
    CheckNotNull(message);

    reinterpret_cast<const LinMessageContainer*>(messageContainer)->WriteTo(*reinterpret_cast<LinMessage*>(message));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteLinMessageToMessageContainer(const DsVeosCoSim_LinMessage* message, DsVeosCoSim_LinMessageContainer* messageContainer) {
    CheckNotNull(message);
    CheckNotNull(messageContainer);

    reinterpret_cast<const LinMessage*>(message)->WriteTo(*reinterpret_cast<LinMessageContainer*>(messageContainer));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteFrMessageContainerToMessage(const DsVeosCoSim_FrMessageContainer* messageContainer, DsVeosCoSim_FrMessage* message) {
    CheckNotNull(messageContainer);
    CheckNotNull(message);

    reinterpret_cast<const FrMessageContainer*>(messageContainer)->WriteTo(*reinterpret_cast<FrMessage*>(message));

    return DsVeosCoSim_Result_Ok;
}

DsVeosCoSim_Result DsVeosCoSim_WriteFrMessageToMessageContainer(const DsVeosCoSim_FrMessage* message, DsVeosCoSim_FrMessageContainer* messageContainer) {
    CheckNotNull(message);
    CheckNotNull(messageContainer);

    reinterpret_cast<const FrMessage*>(message)->WriteTo(*reinterpret_cast<FrMessageContainer*>(messageContainer));

    return DsVeosCoSim_Result_Ok;
}

std::string DsVeosCoSim_SimulationTimeToString(DsVeosCoSim_SimulationTime simulationTime) {
    return format_as(SimulationTime(simulationTime));
}

std::string DsVeosCoSim_IoSignalToString(const DsVeosCoSim_IoSignal* ioSignal) {
    return format_as(*reinterpret_cast<const IoSignal*>(ioSignal));
}

std::string DsVeosCoSim_CanControllerToString(const DsVeosCoSim_CanController* controller) {
    return format_as(*reinterpret_cast<const CanController*>(controller));
}

std::string DsVeosCoSim_EthControllerToString(const DsVeosCoSim_EthController* controller) {
    return format_as(*reinterpret_cast<const EthController*>(controller));
}

std::string DsVeosCoSim_LinControllerToString(const DsVeosCoSim_LinController* controller) {
    return format_as(*reinterpret_cast<const LinController*>(controller));
}

std::string DsVeosCoSim_FrControllerToString(const DsVeosCoSim_FrController* controller) {
    return format_as(*reinterpret_cast<const FrController*>(controller));
}

std::string DsVeosCoSim_ValueToString(DsVeosCoSim_DataType dataType, uint32_t length, const void* value) {
    return ValueToString(static_cast<DataType>(dataType), length, value);
}

std::string DsVeosCoSim_DataToString(const uint8_t* data, size_t dataLength, char separator) {
    return DataToString(data, dataLength, separator);
}

std::string DsVeosCoSim_IoDataToString(const DsVeosCoSim_IoSignal* ioSignal, uint32_t length, const void* value) {
    return IoDataToString(*reinterpret_cast<const IoSignal*>(ioSignal), length, value);
}

std::string DsVeosCoSim_CanMessageToString(const DsVeosCoSim_CanMessage* message) {
    return format_as(*reinterpret_cast<const CanMessage*>(message));
}

std::string DsVeosCoSim_EthMessageToString(const DsVeosCoSim_EthMessage* message) {
    return format_as(*reinterpret_cast<const EthMessage*>(message));
}

std::string DsVeosCoSim_LinMessageToString(const DsVeosCoSim_LinMessage* message) {
    return format_as(*reinterpret_cast<const LinMessage*>(message));
}

std::string DsVeosCoSim_FrMessageToString(const DsVeosCoSim_FrMessage* message) {
    return format_as(*reinterpret_cast<const FrMessage*>(message));
}

std::string DsVeosCoSim_CanMessageContainerToString(const DsVeosCoSim_CanMessageContainer* messageContainer) {
    return format_as(*reinterpret_cast<const CanMessageContainer*>(messageContainer));
}

std::string DsVeosCoSim_EthMessageContainerToString(const DsVeosCoSim_EthMessageContainer* messageContainer) {
    return format_as(*reinterpret_cast<const EthMessageContainer*>(messageContainer));
}

std::string DsVeosCoSim_LinMessageContainerToString(const DsVeosCoSim_LinMessageContainer* messageContainer) {
    return format_as(*reinterpret_cast<const LinMessageContainer*>(messageContainer));
}

std::string DsVeosCoSim_FrMessageContainerToString(const DsVeosCoSim_FrMessageContainer* messageContainer) {
    return format_as(*reinterpret_cast<const FrMessageContainer*>(messageContainer));
}

std::string DsVeosCoSim_CanMessageFlagsToString(DsVeosCoSim_CanMessageFlags flags) {
    return format_as(static_cast<CanMessageFlags>(flags));
}

std::string DsVeosCoSim_EthMessageFlagsToString(DsVeosCoSim_EthMessageFlags flags) {
    return format_as(static_cast<EthMessageFlags>(flags));
}

std::string DsVeosCoSim_LinMessageFlagsToString(DsVeosCoSim_LinMessageFlags flags) {
    return format_as(static_cast<LinMessageFlags>(flags));
}

std::string DsVeosCoSim_FrMessageFlagsToString(DsVeosCoSim_FrMessageFlags flags) {
    return format_as(static_cast<FrMessageFlags>(flags));
}

static_assert(CanMessageMaxLength == DSVEOSCOSIM_CAN_MESSAGE_MAX_LENGTH);
static_assert(EthMessageMaxLength == DSVEOSCOSIM_ETH_MESSAGE_MAX_LENGTH);
static_assert(LinMessageMaxLength == DSVEOSCOSIM_LIN_MESSAGE_MAX_LENGTH);
static_assert(FrMessageMaxLength == DSVEOSCOSIM_FLEXRAY_MESSAGE_MAX_LENGTH);
static_assert(EthAddressLength == DSVEOSCOSIM_ETH_ADDRESS_LENGTH);

static_assert(sizeof(void*) == sizeof(DsVeosCoSim_Handle));

static_assert(sizeof(SimulationTime) == sizeof(DsVeosCoSim_SimulationTime));

static_assert(sizeof(Result) == sizeof(DsVeosCoSim_Result));
static_assert(static_cast<int>(Result::Ok) == DsVeosCoSim_Result_Ok);
static_assert(static_cast<int>(Result::Error) == DsVeosCoSim_Result_Error);
static_assert(static_cast<int>(Result::Empty) == DsVeosCoSim_Result_Empty);
static_assert(static_cast<int>(Result::Full) == DsVeosCoSim_Result_Full);
static_assert(static_cast<int>(Result::InvalidArgument) == DsVeosCoSim_Result_InvalidArgument);
static_assert(static_cast<int>(Result::Disconnected) == DsVeosCoSim_Result_Disconnected);

static_assert(sizeof(CoSimType) == sizeof(uint32_t));

static_assert(sizeof(ConnectionKind) == sizeof(uint32_t));

static_assert(sizeof(Command) == sizeof(DsVeosCoSim_Command));
static_assert(static_cast<int>(Command::None) == DsVeosCoSim_Command_None);
static_assert(static_cast<int>(Command::Step) == DsVeosCoSim_Command_Step);
static_assert(static_cast<int>(Command::Start) == DsVeosCoSim_Command_Start);
static_assert(static_cast<int>(Command::Stop) == DsVeosCoSim_Command_Stop);
static_assert(static_cast<int>(Command::Terminate) == DsVeosCoSim_Command_Terminate);
static_assert(static_cast<int>(Command::Pause) == DsVeosCoSim_Command_Pause);
static_assert(static_cast<int>(Command::Continue) == DsVeosCoSim_Command_Continue);
static_assert(static_cast<int>(Command::TerminateFinished) == DsVeosCoSim_Command_TerminateFinished);
static_assert(static_cast<int>(Command::Ping) == DsVeosCoSim_Command_Ping);

static_assert(sizeof(Severity) == sizeof(DsVeosCoSim_Severity));
static_assert(static_cast<int>(Severity::Error) == DsVeosCoSim_Severity_Error);
static_assert(static_cast<int>(Severity::Warning) == DsVeosCoSim_Severity_Warning);
static_assert(static_cast<int>(Severity::Info) == DsVeosCoSim_Severity_Info);
static_assert(static_cast<int>(Severity::Trace) == DsVeosCoSim_Severity_Trace);

static_assert(sizeof(TerminateReason) == sizeof(DsVeosCoSim_TerminateReason));
static_assert(static_cast<int>(TerminateReason::Finished) == DsVeosCoSim_TerminateReason_Finished);
static_assert(static_cast<int>(TerminateReason::Error) == DsVeosCoSim_TerminateReason_Error);

static_assert(sizeof(ConnectionState) == sizeof(DsVeosCoSim_ConnectionState));
static_assert(static_cast<int>(ConnectionState::Disconnected) == DsVeosCoSim_ConnectionState_Disconnected);
static_assert(static_cast<int>(ConnectionState::Connected) == DsVeosCoSim_ConnectionState_Connected);

static_assert(sizeof(SimulationState) == sizeof(DsVeosCoSim_SimulationState));
static_assert(static_cast<int>(SimulationState::Unloaded) == DsVeosCoSim_SimulationState_Unloaded);
static_assert(static_cast<int>(SimulationState::Stopped) == DsVeosCoSim_SimulationState_Stopped);
static_assert(static_cast<int>(SimulationState::Running) == DsVeosCoSim_SimulationState_Running);
static_assert(static_cast<int>(SimulationState::Paused) == DsVeosCoSim_SimulationState_Paused);
static_assert(static_cast<int>(SimulationState::Terminated) == DsVeosCoSim_SimulationState_Terminated);

static_assert(sizeof(Mode) == sizeof(uint32_t));

static_assert(sizeof(IoSignalId) == sizeof(DsVeosCoSim_IoSignalId));

static_assert(sizeof(DataType) == sizeof(DsVeosCoSim_DataType));
static_assert(static_cast<int>(DataType::Bool) == DsVeosCoSim_DataType_Bool);
static_assert(static_cast<int>(DataType::Int8) == DsVeosCoSim_DataType_Int8);
static_assert(static_cast<int>(DataType::Int16) == DsVeosCoSim_DataType_Int16);
static_assert(static_cast<int>(DataType::Int32) == DsVeosCoSim_DataType_Int32);
static_assert(static_cast<int>(DataType::Int64) == DsVeosCoSim_DataType_Int64);
static_assert(static_cast<int>(DataType::UInt8) == DsVeosCoSim_DataType_UInt8);
static_assert(static_cast<int>(DataType::UInt16) == DsVeosCoSim_DataType_UInt16);
static_assert(static_cast<int>(DataType::UInt32) == DsVeosCoSim_DataType_UInt32);
static_assert(static_cast<int>(DataType::UInt64) == DsVeosCoSim_DataType_UInt64);
static_assert(static_cast<int>(DataType::Float32) == DsVeosCoSim_DataType_Float32);
static_assert(static_cast<int>(DataType::Float64) == DsVeosCoSim_DataType_Float64);

static_assert(sizeof(SizeKind) == sizeof(DsVeosCoSim_SizeKind));
static_assert(static_cast<int>(SizeKind::Fixed) == DsVeosCoSim_SizeKind_Fixed);
static_assert(static_cast<int>(SizeKind::Variable) == DsVeosCoSim_SizeKind_Variable);

static_assert(sizeof(BusControllerId) == sizeof(DsVeosCoSim_BusControllerId));

static_assert(sizeof(BusMessageId) == sizeof(uint32_t));

static_assert(sizeof(LinControllerType) == sizeof(DsVeosCoSim_LinControllerType));
static_assert(static_cast<int>(LinControllerType::Responder) == DsVeosCoSim_LinControllerType_Responder);
static_assert(static_cast<int>(LinControllerType::Commander) == DsVeosCoSim_LinControllerType_Commander);

static_assert(sizeof(CanMessageFlags) == sizeof(DsVeosCoSim_CanMessageFlags));
static_assert(static_cast<int>(CanMessageFlags::Loopback) == DsVeosCoSim_CanMessageFlags_Loopback);
static_assert(static_cast<int>(CanMessageFlags::Error) == DsVeosCoSim_CanMessageFlags_Error);
static_assert(static_cast<int>(CanMessageFlags::Drop) == DsVeosCoSim_CanMessageFlags_Drop);
static_assert(static_cast<int>(CanMessageFlags::ExtendedId) == DsVeosCoSim_CanMessageFlags_ExtendedId);
static_assert(static_cast<int>(CanMessageFlags::BitRateSwitch) == DsVeosCoSim_CanMessageFlags_BitRateSwitch);
static_assert(static_cast<int>(CanMessageFlags::FlexibleDataRateFormat) == DsVeosCoSim_CanMessageFlags_FlexibleDataRateFormat);

static_assert(sizeof(EthMessageFlags) == sizeof(DsVeosCoSim_EthMessageFlags));
static_assert(static_cast<int>(EthMessageFlags::Loopback) == DsVeosCoSim_EthMessageFlags_Loopback);
static_assert(static_cast<int>(EthMessageFlags::Error) == DsVeosCoSim_EthMessageFlags_Error);
static_assert(static_cast<int>(EthMessageFlags::Drop) == DsVeosCoSim_EthMessageFlags_Drop);

static_assert(sizeof(LinMessageFlags) == sizeof(DsVeosCoSim_LinMessageFlags));
static_assert(static_cast<int>(LinMessageFlags::Loopback) == DsVeosCoSim_LinMessageFlags_Loopback);
static_assert(static_cast<int>(LinMessageFlags::Error) == DsVeosCoSim_LinMessageFlags_Error);
static_assert(static_cast<int>(LinMessageFlags::Drop) == DsVeosCoSim_LinMessageFlags_Drop);
static_assert(static_cast<int>(LinMessageFlags::Header) == DsVeosCoSim_LinMessageFlags_Header);
static_assert(static_cast<int>(LinMessageFlags::Response) == DsVeosCoSim_LinMessageFlags_Response);
static_assert(static_cast<int>(LinMessageFlags::WakeEvent) == DsVeosCoSim_LinMessageFlags_WakeEvent);
static_assert(static_cast<int>(LinMessageFlags::SleepEvent) == DsVeosCoSim_LinMessageFlags_SleepEvent);
static_assert(static_cast<int>(LinMessageFlags::EnhancedChecksum) == DsVeosCoSim_LinMessageFlags_EnhancedChecksum);
static_assert(static_cast<int>(LinMessageFlags::TransferOnce) == DsVeosCoSim_LinMessageFlags_TransferOnce);
static_assert(static_cast<int>(LinMessageFlags::ParityFailure) == DsVeosCoSim_LinMessageFlags_ParityFailure);
static_assert(static_cast<int>(LinMessageFlags::Collision) == DsVeosCoSim_LinMessageFlags_Collision);
static_assert(static_cast<int>(LinMessageFlags::NoResponse) == DsVeosCoSim_LinMessageFlags_NoResponse);

static_assert(sizeof(FrMessageFlags) == sizeof(DsVeosCoSim_FrMessageFlags));
static_assert(static_cast<int>(FrMessageFlags::Startup) == DsVeosCoSim_FrMessageFlags_Startup);
static_assert(static_cast<int>(FrMessageFlags::SyncFrame) == DsVeosCoSim_FrMessageFlags_SyncFrame);
static_assert(static_cast<int>(FrMessageFlags::NullFrame) == DsVeosCoSim_FrMessageFlags_NullFrame);
static_assert(static_cast<int>(FrMessageFlags::PayloadPreamble) == DsVeosCoSim_FrMessageFlags_PayloadPreamble);
static_assert(static_cast<int>(FrMessageFlags::Loopback) == DsVeosCoSim_FrMessageFlags_Loopback);
static_assert(static_cast<int>(FrMessageFlags::TransferOnce) == DsVeosCoSim_FrMessageFlags_TransferOnce);
static_assert(static_cast<int>(FrMessageFlags::ChannelA) == DsVeosCoSim_FrMessageFlags_ChannelA);
static_assert(static_cast<int>(FrMessageFlags::ChannelB) == DsVeosCoSim_FrMessageFlags_ChannelB);
static_assert(static_cast<int>(FrMessageFlags::Error) == DsVeosCoSim_FrMessageFlags_Error);
static_assert(static_cast<int>(FrMessageFlags::Drop) == DsVeosCoSim_FrMessageFlags_Drop);

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
