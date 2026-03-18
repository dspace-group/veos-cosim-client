# DsVeosCoSim_FrMessageReceivedCallback

[⬆️ Go to Function Pointers](function-pointers.md)

- [DsVeosCoSim\_FrMessageReceivedCallback](#dsveoscosim_frmessagereceivedcallback)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

Called when a new FlexRay message is received from the VEOS CoSim server.

The `DsVeosCoSim_FrMessageReceivedCallback` callback can be registered with the [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md) function or the [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md) function.

> [!NOTE]
> If the `DsVeosCoSim_FrMessageReceivedCallback` callback is registered, you cannot collect FlexRay messages using the
> [DsVeosCoSim_ReceiveFrMessage](../functions/DsVeosCoSim_ReceiveFrMessage.md) function or the
> [DsVeosCoSim_ReceiveFrMessageContainer](../functions/DsVeosCoSim_ReceiveFrMessageContainer.md).

> [!NOTE]
> If both, the [DsVeosCoSim_FrMessageContainerReceivedCallback](DsVeosCoSim_FrMessageContainerReceivedCallback.md) callback as well as the `DsVeosCoSim_FrMessageReceivedCallback` callback are registered, only the [DsVeosCoSim_FrMessageContainerReceivedCallback](DsVeosCoSim_FrMessageContainerReceivedCallback.md) will be called.

However, if the callback is not registered, each received message is buffered. Currently, the buffer size is `512` messages.
If the buffer is full, new messages are discarded.

## Syntax

```c
typedef void (*DsVeosCoSim_FrMessageReceivedCallback)(
    DsVeosCoSim_SimulationTime simulationTime,
    const DsVeosCoSim_FrController* frController,
    const DsVeosCoSim_FrMessage* message,
    void* userData
);
```

## Parameters

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The current simulation time.

> const [DsVeosCoSim_FrController](../structures/DsVeosCoSim_FrController.md)* frController

The FlexRay controller that sent the message.

> const [DsVeosCoSim_FrMessage](../structures/DsVeosCoSim_FrMessage.md)* message

A pointer to the received FlexRay message.

> void* userData

The user data passed to the co-simulation function via the [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md) structure. Can be `NULL`.

## Return values

This function has no return values.

## See Also

- [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md)
- [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md)
- [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md)
- [DsVeosCoSim_FrMessageContainerReceivedCallback](DsVeosCoSim_FrMessageContainerReceivedCallback.md)
