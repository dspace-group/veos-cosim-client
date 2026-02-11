# DsVeosCoSim_Result

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_Result](#dsveoscosim_result)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains the possible return values of VEOS CoSim functions.

## Syntax

```c
typedef enum DsVeosCoSim_Result {
    DsVeosCoSim_Result_Ok,
    DsVeosCoSim_Result_Error,
    DsVeosCoSim_Result_Empty,
    DsVeosCoSim_Result_Full
    DsVeosCoSim_Result_InvalidArgument,
    DsVeosCoSim_Result_Disconnected
} DsVeosCoSim_Result;
```

## Values

> DsVeosCoSim_Result_Ok

The function call was successful.

> DsVeosCoSim_Result_Error

The function call failed with an error. In this case, a log message is sent via the [DsVeosCoSim_LogCallback](../function-pointers/DsVeosCoSim_LogCallback.md) callback.

> DsVeosCoSim_Result_Empty

Only for bus message receive functions. Indicates that no bus message was found in the internal buffer.

> DsVeosCoSim_Result_Full

Only for bus message transmit functions. Indicates that no space is left for new bus messages in the internal buffer.

> DsVeosCoSim_Result_InvalidArgument

The function call failed due to an invalid argument.

> DsVeosCoSim_Result_Disconnected

The function detected a disconnection from the VEOS CoSim server.

## See Also

- [DsVeosCoSim_Connect](../functions/DsVeosCoSim_Connect.md)
- [DsVeosCoSim_Disconnect](../functions/DsVeosCoSim_Disconnect.md)
- [DsVeosCoSim_GetConnectionState](../functions/DsVeosCoSim_GetConnectionState.md)
- [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md)
- [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md)
- [DsVeosCoSim_PollCommand](../functions/DsVeosCoSim_PollCommand.md)
- [DsVeosCoSim_FinishCommand](../functions/DsVeosCoSim_FinishCommand.md)
- [DsVeosCoSim_SetNextSimulationTime](../functions/DsVeosCoSim_SetNextSimulationTime.md)
- [DsVeosCoSim_GetStepSize](../functions/DsVeosCoSim_GetStepSize.md)
- [DsVeosCoSim_GetIncomingSignals](../functions/DsVeosCoSim_GetIncomingSignals.md)
- [DsVeosCoSim_ReadIncomingSignal](../functions/DsVeosCoSim_ReadIncomingSignal.md)
- [DsVeosCoSim_GetOutgoingSignals](../functions/DsVeosCoSim_GetOutgoingSignals.md)
- [DsVeosCoSim_WriteOutgoingSignal](../functions/DsVeosCoSim_WriteOutgoingSignal.md)
- [DsVeosCoSim_GetCanControllers](../functions/DsVeosCoSim_GetCanControllers.md)
- [DsVeosCoSim_ReceiveCanMessage](../functions/DsVeosCoSim_ReceiveCanMessage.md)
- [DsVeosCoSim_ReceiveCanMessageContainer](../functions/DsVeosCoSim_ReceiveCanMessageContainer.md)
- [DsVeosCoSim_TransmitCanMessage](../functions/DsVeosCoSim_TransmitCanMessage.md)
- [DsVeosCoSim_TransmitCanMessageContainer](../functions/DsVeosCoSim_TransmitCanMessageContainer.md)
- [DsVeosCoSim_GetEthControllers](../functions/DsVeosCoSim_GetEthControllers.md)
- [DsVeosCoSim_ReceiveEthMessage](../functions/DsVeosCoSim_ReceiveEthMessage.md)
- [DsVeosCoSim_ReceiveEthMessageContainer](../functions/DsVeosCoSim_ReceiveEthMessageContainer.md)
- [DsVeosCoSim_TransmitEthMessage](../functions/DsVeosCoSim_TransmitEthMessage.md)
- [DsVeosCoSim_TransmitEthMessageContainer](../functions/DsVeosCoSim_TransmitEthMessageContainer.md)
- [DsVeosCoSim_GetLinControllers](../functions/DsVeosCoSim_GetLinControllers.md)
- [DsVeosCoSim_ReceiveLinMessage](../functions/DsVeosCoSim_ReceiveLinMessage.md)
- [DsVeosCoSim_ReceiveLinMessageContainer](../functions/DsVeosCoSim_ReceiveLinMessageContainer.md)
- [DsVeosCoSim_TransmitLinMessage](../functions/DsVeosCoSim_TransmitLinMessage.md)
- [DsVeosCoSim_TransmitLinMessageContainer](../functions/DsVeosCoSim_TransmitLinMessageContainer.md)
- [DsVeosCoSim_GetFrControllers](../functions/DsVeosCoSim_GetFrControllers.md)
- [DsVeosCoSim_ReceiveFrMessage](../functions/DsVeosCoSim_ReceiveFrMessage.md)
- [DsVeosCoSim_ReceiveFrMessageContainer](../functions/DsVeosCoSim_ReceiveFrMessageContainer.md)
- [DsVeosCoSim_TransmitFrMessage](../functions/DsVeosCoSim_TransmitFrMessage.md)
- [DsVeosCoSim_TransmitFrMessageContainer](../functions/DsVeosCoSim_TransmitFrMessageContainer.md)
