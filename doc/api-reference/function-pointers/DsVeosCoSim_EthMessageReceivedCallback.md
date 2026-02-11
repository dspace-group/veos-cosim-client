# DsVeosCoSim_EthMessageReceivedCallback

[⬆️ Go to Function Pointers](function-pointers.md)

- [DsVeosCoSim\_EthMessageReceivedCallback](#dsveoscosim_ethmessagereceivedcallback)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

Called when a new Ethernet message is received from the VEOS CoSim server.

The `DsVeosCoSim_EthMessageReceivedCallback` callback can be registered with the [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md) function or the [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md) function.

> [!NOTE]
> If the `DsVeosCoSim_EthMessageReceivedCallback` callback is registered, you cannot collect Ethernet messages using the
> [DsVeosCoSim_ReceiveEthMessage](../functions/DsVeosCoSim_ReceiveEthMessage.md) function or the
> [DsVeosCoSim_ReceiveEthMessageContainer](../functions/DsVeosCoSim_ReceiveEthMessageContainer.md).

> [!NOTE]
> If both, the [DsVeosCoSim_EthMessageContainerReceivedCallback](DsVeosCoSim_EthMessageContainerReceivedCallback.md) callback as well as the `DsVeosCoSim_EthMessageReceivedCallback` callback are registered, only the [DsVeosCoSim_EthMessageContainerReceivedCallback](DsVeosCoSim_EthMessageContainerReceivedCallback.md) will be called.

However, if the callback is not registered, each received message is buffered. Currently, the buffer size is `512` messages.
If the buffer is full, new messages are discarded.

## Syntax

```c
typedef void (*DsVeosCoSim_EthMessageReceivedCallback)(
    DsVeosCoSim_SimulationTime simulationTime,
    const DsVeosCoSim_EthController* ethController,
    const DsVeosCoSim_EthMessage* message,
    void* userData
);
```

## Parameters

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The current simulation time.

> const [DsVeosCoSim_EthController](../structures/DsVeosCoSim_EthController.md)* ethController

The Ethernet controller that sent the message.

> const [DsVeosCoSim_EthMessage](../structures/DsVeosCoSim_EthMessage.md)* message

A pointer to the received Ethernet message.

> void* userData

The user data passed to the co-simulation function via the [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md) structure. Can be `NULL`.

## Return values

This function has no return values.

## See Also

- [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md)
- [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md)
- [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md)
- [DsVeosCoSim_EthMessageContainerReceivedCallback](DsVeosCoSim_EthMessageContainerReceivedCallback.md)
