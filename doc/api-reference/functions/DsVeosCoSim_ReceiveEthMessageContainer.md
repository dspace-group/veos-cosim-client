# DsVeosCoSim_ReceiveEthMessageContainer

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_ReceiveEthMessageContainer](#dsveoscosim_receiveethmessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Receives an Ethernet message from the VEOS CoSim server.

> [!NOTE]
> If the [DsVeosCoSim_EthMessageReceivedCallback](DsVeosCoSim_EthMessageReceivedCallback.md) callback
> or the [DsVeosCoSim_EthMessageContainerReceivedCallback](DsVeosCoSim_EthMessageContainerReceivedCallback.md) callback is registered,
> Ethernet messages cannot be collected using the `DsVeosCoSim_ReceiveEthMessageContainer` function.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ReceiveEthMessageContainer(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_EthMessageContainer* messageContainer
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_EthMessageContainer](../structures/DsVeosCoSim_EthMessageContainer.md)* messageContainer

A pointer to the Ethernet message container.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
