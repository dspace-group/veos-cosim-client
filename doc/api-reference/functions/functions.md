# Functions

> [⬆️ Go to DsVeosCoSim Client API Reference](../api-reference.md)

- [Functions](#functions)
  - [Description](#description)
  - [List of Functions](#list-of-functions)

## Description

Describes all available functions.

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

> [DsVeosCoSim_GetCurrentSimulationTime](DsVeosCoSim_GetCurrentSimulationTime.md)

Gets the current simulation time.

> [DsVeosCoSim_GetDataTypeSize](DsVeosCoSim_GetDataTypeSize.md)

Gets the size in bytes for a given data type.

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

> [DsVeosCoSim_GetRoundTripTime](DsVeosCoSim_GetRoundTripTime.md)

Gets the round trip time.

> [DsVeosCoSim_GetSimulationState](DsVeosCoSim_GetSimulationState.md)

Gets the current simulation state.

> [DsVeosCoSim_GetStepSize](DsVeosCoSim_GetStepSize.md)

Gets the step size of the VEOS CoSim server.

> [DsVeosCoSim_LinControllerTypeToString](DsVeosCoSim_LinControllerTypeToString.md)

Converts a LIN controller type to a string.

> [DsVeosCoSim_PollCommand](DsVeosCoSim_PollCommand.md)

Polls the simulator for a command.

> [DsVeosCoSim_CommandToString](DsVeosCoSim_CommandToString.md)

Converts a command to a string.

> [DsVeosCoSim_ConnectionStateToString](DsVeosCoSim_ConnectionStateToString.md)

Converts a connection state to a string.

> [DsVeosCoSim_ContinueSimulation](DsVeosCoSim_ContinueSimulation.md)

Continues the simulation.

> [DsVeosCoSim_DataTypeToString](DsVeosCoSim_DataTypeToString.md)

Converts a data type to a string.

> [DsVeosCoSim_ReadIncomingSignal](DsVeosCoSim_ReadIncomingSignal.md)

Reads a value from an incoming signal of the VEOS CoSim server identified by the given handle.

> [DsVeosCoSim_ReceiveCanMessage](DsVeosCoSim_ReceiveCanMessage.md)

Receives a CAN message from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveCanMessageContainer](DsVeosCoSim_ReceiveCanMessageContainer.md)

Receives a CAN message container from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveEthMessage](DsVeosCoSim_ReceiveEthMessage.md)

Receives an Ethernet message from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveEthMessageContainer](DsVeosCoSim_ReceiveEthMessageContainer.md)

Receives an Ethernet message container from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveFrMessage](DsVeosCoSim_ReceiveFrMessage.md)

Receives a FlexRay message from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveFrMessageContainer](DsVeosCoSim_ReceiveFrMessageContainer.md)

Receives a FlexRay message container from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveLinMessage](DsVeosCoSim_ReceiveLinMessage.md)

Receives a LIN message from the VEOS CoSim server.

> [DsVeosCoSim_ReceiveLinMessageContainer](DsVeosCoSim_ReceiveLinMessageContainer.md)

Receives a LIN message container from the VEOS CoSim server.

> [DsVeosCoSim_RunCallbackBasedCoSimulation](DsVeosCoSim_RunCallbackBasedCoSimulation.md)

Starts a callback-based co-simulation.

> [DsVeosCoSim_PauseSimulation](DsVeosCoSim_PauseSimulation.md)

Pauses the simulation.

> [DsVeosCoSim_ResultToString](DsVeosCoSim_ResultToString.md)

Converts a result value to a string.

> [DsVeosCoSim_SetLogCallback](DsVeosCoSim_SetLogCallback.md)

Specifies the log callback function.

> [DsVeosCoSim_SetNextSimulationTime](DsVeosCoSim_SetNextSimulationTime.md)

Sets the next simulation time for the given client handle.

> [DsVeosCoSim_SeverityToString](DsVeosCoSim_SeverityToString.md)

Converts a severity to a string.

> [DsVeosCoSim_SimulationStateToString](DsVeosCoSim_SimulationStateToString.md)

Converts a simulation state to a string.

> [DsVeosCoSim_SizeKindToString](DsVeosCoSim_SizeKindToString.md)

Converts a size kind to a string.

> [DsVeosCoSim_StartSimulation](DsVeosCoSim_StartSimulation.md)

Starts the simulation.

> [DsVeosCoSim_StartPollingBasedCoSimulation](DsVeosCoSim_StartPollingBasedCoSimulation.md)

Starts a polling-based co-simulation in non-blocking mode.

> [DsVeosCoSim_StopSimulation](DsVeosCoSim_StopSimulation.md)

Stops the simulation.

> [DsVeosCoSim_TerminateReasonToString](DsVeosCoSim_TerminateReasonToString.md)

Converts a terminate reason to a string.

> [DsVeosCoSim_TerminateSimulation](DsVeosCoSim_TerminateSimulation.md)

Terminates the simulation.

> [DsVeosCoSim_TransmitCanMessage](DsVeosCoSim_TransmitCanMessage.md)

Transmits a CAN message to the VEOS CoSim server.

> [DsVeosCoSim_TransmitCanMessageContainer](DsVeosCoSim_TransmitCanMessageContainer.md)

Transmits a CAN message container to the VEOS CoSim server.

> [DsVeosCoSim_TransmitEthMessage](DsVeosCoSim_TransmitEthMessage.md)

Transmits an Ethernet message to the VEOS CoSim server.

> [DsVeosCoSim_TransmitEthMessageContainer](DsVeosCoSim_TransmitEthMessageContainer.md)

Transmits an Ethernet message container to the VEOS CoSim server.

> [DsVeosCoSim_TransmitFrMessage](DsVeosCoSim_TransmitFrMessage.md)

Transmits a FlexRay message to the VEOS CoSim server.

> [DsVeosCoSim_TransmitFrMessageContainer](DsVeosCoSim_TransmitFrMessageContainer.md)

Transmits a FlexRay message container to the VEOS CoSim server.

> [DsVeosCoSim_TransmitLinMessage](DsVeosCoSim_TransmitLinMessage.md)

Transmits a LIN message to the VEOS CoSim server.

> [DsVeosCoSim_TransmitLinMessageContainer](DsVeosCoSim_TransmitLinMessageContainer.md)

Transmits a LIN message container to the VEOS CoSim server.

> [DsVeosCoSim_WriteCanMessageContainerToMessage](DsVeosCoSim_WriteCanMessageContainerToMessage.md)

Converts a CAN message container to a CAN message.

> [DsVeosCoSim_WriteCanMessageToMessageContainer](DsVeosCoSim_WriteCanMessageToMessageContainer.md)

Converts a CAN message to a CAN message container.

> [DsVeosCoSim_WriteEthMessageContainerToMessage](DsVeosCoSim_WriteEthMessageContainerToMessage.md)

Converts an Ethernet message container to an Ethernet message.

> [DsVeosCoSim_WriteEthMessageToMessageContainer](DsVeosCoSim_WriteEthMessageToMessageContainer.md)

Converts an Ethernet message to an Ethernet message container.

> [DsVeosCoSim_WriteFrMessageContainerToMessage](DsVeosCoSim_WriteFrMessageContainerToMessage.md)

Converts a FlexRay message container to a FlexRay message.

> [DsVeosCoSim_WriteFrMessageToMessageContainer](DsVeosCoSim_WriteFrMessageToMessageContainer.md)

Converts a FlexRay message to a FlexRay message container.

> [DsVeosCoSim_WriteLinMessageContainerToMessage](DsVeosCoSim_WriteLinMessageContainerToMessage.md)

Converts a LIN message container to a LIN message.

> [DsVeosCoSim_WriteLinMessageToMessageContainer](DsVeosCoSim_WriteLinMessageToMessageContainer.md)

Converts a LIN message to a LIN message container.

> [DsVeosCoSim_SimulationTimeToString](DsVeosCoSim_SimulationTimeToString.md)

Formats a simulation time as a string.

> [DsVeosCoSim_IoSignalToString](DsVeosCoSim_IoSignalToString.md)

Formats an I/O signal as a string.

> [DsVeosCoSim_CanControllerToString](DsVeosCoSim_CanControllerToString.md)

Formats a CAN controller as a string.

> [DsVeosCoSim_EthControllerToString](DsVeosCoSim_EthControllerToString.md)

Formats an Ethernet controller as a string.

> [DsVeosCoSim_LinControllerToString](DsVeosCoSim_LinControllerToString.md)

Formats a LIN controller as a string.

> [DsVeosCoSim_FrControllerToString](DsVeosCoSim_FrControllerToString.md)

Formats a FlexRay controller as a string.

> [DsVeosCoSim_ValueToString](DsVeosCoSim_ValueToString.md)

Formats a typed value buffer as a string.

> [DsVeosCoSim_DataToString](DsVeosCoSim_DataToString.md)

Formats raw byte data as a string.

> [DsVeosCoSim_IoDataToString](DsVeosCoSim_IoDataToString.md)

Formats I/O signal data as a string.

> [DsVeosCoSim_CanMessageToString](DsVeosCoSim_CanMessageToString.md)

Formats a CAN message as a string.

> [DsVeosCoSim_EthMessageToString](DsVeosCoSim_EthMessageToString.md)

Formats an Ethernet message as a string.

> [DsVeosCoSim_LinMessageToString](DsVeosCoSim_LinMessageToString.md)

Formats a LIN message as a string.

> [DsVeosCoSim_FrMessageToString](DsVeosCoSim_FrMessageToString.md)

Formats a FlexRay message as a string.

> [DsVeosCoSim_CanMessageContainerToString](DsVeosCoSim_CanMessageContainerToString.md)

Formats a CAN message container as a string.

> [DsVeosCoSim_EthMessageContainerToString](DsVeosCoSim_EthMessageContainerToString.md)

Formats an Ethernet message container as a string.

> [DsVeosCoSim_LinMessageContainerToString](DsVeosCoSim_LinMessageContainerToString.md)

Formats a LIN message container as a string.

> [DsVeosCoSim_FrMessageContainerToString](DsVeosCoSim_FrMessageContainerToString.md)

Formats a FlexRay message container as a string.

> [DsVeosCoSim_CanMessageFlagsToString](DsVeosCoSim_CanMessageFlagsToString.md)

Formats CAN message flags as a string.

> [DsVeosCoSim_EthMessageFlagsToString](DsVeosCoSim_EthMessageFlagsToString.md)

Formats Ethernet message flags as a string.

> [DsVeosCoSim_LinMessageFlagsToString](DsVeosCoSim_LinMessageFlagsToString.md)

Formats LIN message flags as a string.

> [DsVeosCoSim_FrMessageFlagsToString](DsVeosCoSim_FrMessageFlagsToString.md)

Formats FlexRay message flags as a string.

> [DsVeosCoSim_WriteOutgoingSignal](DsVeosCoSim_WriteOutgoingSignal.md)

Writes a value to an outgoing signal of the VEOS CoSim server identified by the given handle.
