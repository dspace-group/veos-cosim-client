# DsVeosCoSim_FrMessageContainerReceivedCallback

[⬆️ Go to Function Pointers](function-pointers.md)

- [DsVeosCoSim\_FrMessageContainerReceivedCallback](#dsveoscosim_frmessagecontainerreceivedcallback)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

Called when a new FlexRay message is received from the VEOS CoSim server.

The `DsVeosCoSim_FrMessageContainerReceivedCallback` callback can be registered with the [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md) function or the [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md) function.

> [!NOTE]
> If the `DsVeosCoSim_FrMessageContainerReceivedCallback` callback is registered, you cannot collect FlexRay messages using the
> [DsVeosCoSim_ReceiveFrMessage](../functions/DsVeosCoSim_ReceiveFrMessage.md) function or the
> [DsVeosCoSim_ReceiveFrMessageContainer](../functions/DsVeosCoSim_ReceiveFrMessageContainer.md).

> [!NOTE]
> If both, the `DsVeosCoSim_FrMessageContainerReceivedCallback` callback as well as the [DsVeosCoSim_FrMessageReceivedCallback](DsVeosCoSim_FrMessageReceivedCallback.md) callback are registered, only the `DsVeosCoSim_FrMessageContainerReceivedCallback` will be called.

However, if the callback is not registered, each received message is buffered. Currently, the buffer size is `512` messages.
If the buffer is full, new messages are discarded.

## Syntax

```c
typedef void (*DsVeosCoSim_FrMessageContainerReceivedCallback)(
    DsVeosCoSim_SimulationTime simulationTime,
    const DsVeosCoSim_FrController* frController,
    const DsVeosCoSim_FrMessageContainer* messageContainer,
    void* userData
);
```

## Parameters

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The current simulation time.

> const [DsVeosCoSim_FrController](../structures/DsVeosCoSim_FrController.md)* frController

The FlexRay controller that sent the message.

> const [DsVeosCoSim_FrMessageContainer](../structures/DsVeosCoSim_FrMessageContainer.md)* messageContainer

A pointer to the received FlexRay message container.

> void* userData

The user data passed to the co-simulation function via the [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md) structure. Fr be `NULL`.

## Return values

This function has no return values.

## See Also

- [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md)
- [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md)
- [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md)
- [DsVeosCoSim_FrMessageReceivedCallback](DsVeosCoSim_FrMessageReceivedCallback.md)
