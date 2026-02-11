# DsVeosCoSim_LinMessageReceivedCallback

[⬆️ Go to Function Pointers](function-pointers.md)

- [DsVeosCoSim\_LinMessageReceivedCallback](#dsveoscosim_linmessagereceivedcallback)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

Called when a new LIN message is received from the VEOS CoSim server.

The `DsVeosCoSim_LinMessageReceivedCallback` callback can be registered with the [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md) function or the [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md) function.

> [!NOTE]
> If the `DsVeosCoSim_LinMessageReceivedCallback` callback is registered, you cannot collect LIN messages using the
> [DsVeosCoSim_ReceiveLinMessage](../functions/DsVeosCoSim_ReceiveLinMessage.md) function or the
> [DsVeosCoSim_ReceiveLinMessageContainer](../functions/DsVeosCoSim_ReceiveLinMessageContainer.md).

> [!NOTE]
> If both, the [DsVeosCoSim_LinMessageContainerReceivedCallback](DsVeosCoSim_LinMessageContainerReceivedCallback.md) callback as well as the `DsVeosCoSim_LinMessageReceivedCallback` callback are registered, only the [DsVeosCoSim_LinMessageContainerReceivedCallback](DsVeosCoSim_LinMessageContainerReceivedCallback.md) will be called.

However, if the callback is not registered, each received message is buffered. Currently, the buffer size is `512` messages.
If the buffer is full, new messages are discarded.

## Syntax

```c
typedef void (*DsVeosCoSim_LinMessageReceivedCallback)(
    DsVeosCoSim_SimulationTime simulationTime,
    const DsVeosCoSim_LinController* ethController,
    const DsVeosCoSim_LinMessage* message,
    void* userData
);
```

## Parameters

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The current simulation time.

> const [DsVeosCoSim_LinController](../structures/DsVeosCoSim_LinController.md)* ethController

The LIN controller that sent the message.

> const [DsVeosCoSim_LinMessage](../structures/DsVeosCoSim_LinMessage.md)* message

A pointer to the received LIN message.

> void* userData

The user data passed to the co-simulation function via the [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md) structure. Can be `NULL`.

## Return values

This function has no return values.

## See Also

- [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md)
- [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md)
- [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md)
- [DsVeosCoSim_LinMessageContainerReceivedCallback](DsVeosCoSim_LinMessageContainerReceivedCallback.md)
