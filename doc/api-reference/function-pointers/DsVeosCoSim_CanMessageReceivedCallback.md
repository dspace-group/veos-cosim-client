# DsVeosCoSim_CanMessageReceivedCallback

[⬆️ Go to Function Pointers](function-pointers.md)

- [DsVeosCoSim\_CanMessageReceivedCallback](#dsveoscosim_canmessagereceivedcallback)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

Called when a new CAN message is received from the VEOS CoSim server.

The `DsVeosCoSim_CanMessageReceivedCallback` callback can be registered with the [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md) function or the [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md) function.

> [!NOTE]
> If the `DsVeosCoSim_CanMessageReceivedCallback` callback is registered, you cannot collect CAN messages using the
> [DsVeosCoSim_ReceiveCanMessage](../functions/DsVeosCoSim_ReceiveCanMessage.md) function or the
> [DsVeosCoSim_ReceiveCanMessageContainer](../functions/DsVeosCoSim_ReceiveCanMessageContainer.md).

> [!NOTE]
> If both, the [DsVeosCoSim_CanMessageContainerReceivedCallback](DsVeosCoSim_CanMessageContainerReceivedCallback.md) callback as well as the `DsVeosCoSim_CanMessageReceivedCallback` callback are registered, only the [DsVeosCoSim_CanMessageContainerReceivedCallback](DsVeosCoSim_CanMessageContainerReceivedCallback.md) will be called.

However, if the callback is not registered, each received message is buffered. Currently, the buffer size is `512` messages.
If the buffer is full, new messages are discarded.

## Syntax

```c
typedef void (*DsVeosCoSim_CanMessageReceivedCallback)(
    DsVeosCoSim_SimulationTime simulationTime,
    const DsVeosCoSim_CanController* canController,
    const DsVeosCoSim_CanMessage* message,
    void* userData
);
```

## Parameters

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The current simulation time.

> const [DsVeosCoSim_CanController](../structures/DsVeosCoSim_CanController.md)* canController

The CAN controller that sent the message.

> const [DsVeosCoSim_CanMessage](../structures/DsVeosCoSim_CanMessage.md)* message

A pointer to the received CAN message

> void* userData

The user data passed to the co-simulation function via the[DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md) structure. Can be `NULL`.

## Return values

This function has no return values.

## See Also

- [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md)
- [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md)
- [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md)
- [DsVeosCoSim_CanMessageContainerReceivedCallback](DsVeosCoSim_CanMessageContainerReceivedCallback.md)
