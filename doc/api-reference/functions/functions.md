# Functions

> [⬆️ Go to DsVeosCoSim Client API Reference](../api-reference.md)

- [Functions](#functions)
  - [Descriptions](#descriptions)
  - [List of Functions](#list-of-functions)

## Descriptions

Describes all available functions and function pointers.

## List of Functions

> [DsVeosCoSim_Connect](DsVeosCoSim_Connect.md)

Connects the VEOS CoSim client to the VEOS CoSim server.

> [DsVeosCoSim_Create](DsVeosCoSim_Create.md)

Creates a new VEOS CoSim client.

> [DsVeosCoSim_Destroy](DsVeosCoSim_Destroy.md)

Destroys the given handle.

> [DsVeosCoSim_Disconnect](DsVeosCoSim_Disconnect.md)

Disconnects the VEOS CoSim client from the VEOS CoSim server.

> [DsVeosCoSim_FinishCommand](DsVeosCoSim_FinishCommand.md)

Finishes the current command in a polling-based simulation.

> [DsVeosCoSim_GetCanControllers](DsVeosCoSim_GetCanControllers.md)

Gets all available CAN controllers in the co-simulation.

> [DsVeosCoSim_GetConnectionState](DsVeosCoSim_GetConnectionState.md)

Gets the connection state for a given client handle.

> [DsVeosCoSim_GetEthControllers](DsVeosCoSim_GetEthControllers.md)

Gets all available Ethernet controllers in the co-simulation.

> [DsVeosCoSim_GetFrControllers](DsVeosCoSim_GetFrControllers.md)

Gets all available FlexRay controllers in the co-simulation.

> [DsVeosCoSim_GetIncomingSignals](DsVeosCoSim_GetIncomingSignals.md)

Gets all available incoming signals.

> [DsVeosCoSim_GetLinControllers](DsVeosCoSim_GetLinControllers.md)

Gets all available LIN controllers in the co-simulation.

> [DsVeosCoSim_GetOutgoingSignals](DsVeosCoSim_GetOutgoingSignals.md)

Gets all available outgoing signals.

> [DsVeosCoSim_GetStepSize](DsVeosCoSim_GetStepSize.md)

Gets the step size of the VEOS CoSim server.

> [DsVeosCoSim_PollCommand](DsVeosCoSim_PollCommand.md)

Polls the simulator for a command.

> [DsVeosCoSim_ReadIncomingSignal](DsVeosCoSim_ReadIncomingSignal.md)

Reads a value from an incoming signal of the VEOS CoSim server identified by the given handle.

> [DsVeosCoSim_ReceiveCanMessage](DsVeosCoSim_ReceiveCanMessage.md)

Receives a CAN message from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveCanMessageContainer](DsVeosCoSim_ReceiveCanMessageContainer.md)

Receives a CAN message from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveEthMessage](DsVeosCoSim_ReceiveEthMessage.md)

Receives an Ethernet message from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveEthMessageContainer](DsVeosCoSim_ReceiveEthMessageContainer.md)

Receives an Ethernet message from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveFrMessage](DsVeosCoSim_ReceiveFrMessage.md)

Receives an FlexRay message from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveFrMessageContainer](DsVeosCoSim_ReceiveFrMessageContainer.md)

Receives an FlexRay message from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveLinMessage](DsVeosCoSim_ReceiveLinMessage.md)

Receives a LIN message from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveLinMessageContainer](DsVeosCoSim_ReceiveLinMessageContainer.md)

Receives a LIN message from the VEOS CoSim server.

> [DsVeosCoSim_RunCallbackBasedCoSimulation](DsVeosCoSim_RunCallbackBasedCoSimulation.md)

Starts a callback-based co-simulation.

> [DsVeosCoSim_SetLogCallback](DsVeosCoSim_SetLogCallback.md)

Specifies the log callback function.

> [DsVeosCoSim_SetNextSimulationTime](DsVeosCoSim_SetNextSimulationTime.md)

Sets the next simulation time for the given client handle.

> [DsVeosCoSim_StartPollingBasedCoSimulation](DsVeosCoSim_StartPollingBasedCoSimulation.md)

Starts a co-simulation in non-blocking mode.

> [DsVeosCoSim_TransmitCanMessage](DsVeosCoSim_TransmitCanMessage.md)

Transmits a CAN message to the VEOS CoSim server.

> [DsVeosCoSim_TransmitCanMessageContainer](DsVeosCoSim_TransmitCanMessageContainer.md)

Transmits a CAN message to the VEOS CoSim server.

> [DsVeosCoSim_TransmitEthMessage](DsVeosCoSim_TransmitEthMessage.md)

Transmits an Ethernet message to the VEOS CoSim server.

> [DsVeosCoSim_TransmitEthMessageContainer](DsVeosCoSim_TransmitEthMessageContainer.md)

Transmits an Ethernet message to the VEOS CoSim server.

> [DsVeosCoSim_TransmitFrMessage](DsVeosCoSim_TransmitFrMessage.md)

Transmits an FlexRay message to the VEOS CoSim server.

> [DsVeosCoSim_TransmitFrMessageContainer](DsVeosCoSim_TransmitFrMessageContainer.md)

Transmits an FlexRay message to the VEOS CoSim server.

> [DsVeosCoSim_TransmitLinMessage](DsVeosCoSim_TransmitLinMessage.md)

Transmits a LIN message to the VEOS CoSim server.

> [DsVeosCoSim_TransmitLinMessageContainer](DsVeosCoSim_TransmitLinMessageContainer.md)

Transmits a LIN message to the VEOS CoSim server.

> [DsVeosCoSim_WriteOutgoingSignal](DsVeosCoSim_WriteOutgoingSignal.md)

Writes a value to an outgoing signal of the VEOS CoSim server identified by the given handle.
