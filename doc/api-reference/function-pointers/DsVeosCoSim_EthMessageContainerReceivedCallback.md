# DsVeosCoSim_EthMessageContainerReceivedCallback

[⬆️ Go to Function Pointers](function-pointers.md)

- [DsVeosCoSim\_EthMessageContainerReceivedCallback](#dsveoscosim_ethmessagecontainerreceivedcallback)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

Called when a new Ethernet message is received from the VEOS CoSim server.

The `DsVeosCoSim_EthMessageContainerReceivedCallback` callback can be registered with the [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md) function or the [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md) function.

> [!NOTE]
> If the `DsVeosCoSim_EthMessageContainerReceivedCallback` callback is registered, you cannot collect Ethernet messages using the
> [DsVeosCoSim_ReceiveEthMessage](../functions/DsVeosCoSim_ReceiveEthMessage.md) function or the
> [DsVeosCoSim_ReceiveEthMessageContainer](../functions/DsVeosCoSim_ReceiveEthMessageContainer.md).

> [!NOTE]
> If both, the `DsVeosCoSim_EthMessageContainerReceivedCallback` callback as well as the [DsVeosCoSim_EthMessageReceivedCallback](DsVeosCoSim_EthMessageReceivedCallback.md) callback are registered, only the `DsVeosCoSim_EthMessageContainerReceivedCallback` will be called.

However, if the callback is not registered, each received message is buffered. Currently, the buffer size is `512` messages.
If the buffer is full, new messages are discarded.

## Syntax

```c
typedef void (*DsVeosCoSim_EthMessageContainerReceivedCallback)(
    DsVeosCoSim_SimulationTime simulationTime,
    const DsVeosCoSim_EthController* ethController,
    const DsVeosCoSim_EthMessageContainer* messageContainer,
    void* userData
);
```

## Parameters

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The current simulation time.

> const [DsVeosCoSim_EthController](../structures/DsVeosCoSim_EthController.md)* ethController

The Ethernet controller that sent the message.

> const [DsVeosCoSim_EthMessageContainer](../structures/DsVeosCoSim_EthMessageContainer.md)* messageContainer

A pointer to the received Ethernet message container.

> void* userData

The user data passed to the co-simulation function via the [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md) structure. Eth be `NULL`.

## Return values

This function has no return values.

## See Also

- [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md)
- [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md)
- [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md)
- [DsVeosCoSim_EthMessageReceivedCallback](DsVeosCoSim_EthMessageReceivedCallback.md)
