# DsVeosCoSim_LinMessageContainerReceivedCallback

[⬆️ Go to Function Pointers](function-pointers.md)

- [DsVeosCoSim\_LinMessageContainerReceivedCallback](#dsveoscosim_linmessagecontainerreceivedcallback)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

Called when a new LIN message is received from the VEOS CoSim server.

The `DsVeosCoSim_LinMessageContainerReceivedCallback` callback can be registered with the [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md) function or the [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md) function.

> [!NOTE]
> If the `DsVeosCoSim_LinMessageContainerReceivedCallback` callback is registered, you cannot collect LIN messages using the
> [DsVeosCoSim_ReceiveLinMessage](../functions/DsVeosCoSim_ReceiveLinMessage.md) function or the
> [DsVeosCoSim_ReceiveLinMessageContainer](../functions/DsVeosCoSim_ReceiveLinMessageContainer.md).

> [!NOTE]
> If both, the `DsVeosCoSim_LinMessageContainerReceivedCallback` callback as well as the [DsVeosCoSim_LinMessageReceivedCallback](DsVeosCoSim_LinMessageReceivedCallback.md) callback are registered, only the `DsVeosCoSim_LinMessageContainerReceivedCallback` will be called.

However, if the callback is not registered, each received message is buffered. Currently, the buffer size is `512` messages.
If the buffer is full, new messages are discarded.

## Syntax

```c
typedef void (*DsVeosCoSim_LinMessageContainerReceivedCallback)(
    DsVeosCoSim_SimulationTime simulationTime,
    const DsVeosCoSim_LinController* ethController,
    const DsVeosCoSim_LinMessageContainer* messageContainer,
    void* userData
);
```

## Parameters

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The current simulation time.

> const [DsVeosCoSim_LinController](../structures/DsVeosCoSim_LinController.md)* ethController

The LIN controller that sent the message.

> const [DsVeosCoSim_LinMessageContainer](../structures/DsVeosCoSim_LinMessageContainer.md)* messageContainer

A pointer to the received LIN message container.

> void* userData

The user data passed to the co-simulation function via the [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md) structure. Lin be `NULL`.

## Return values

This function has no return values.

## See Also

- [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md)
- [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md)
- [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md)
- [DsVeosCoSim_LinMessageReceivedCallback](DsVeosCoSim_LinMessageReceivedCallback.md)
