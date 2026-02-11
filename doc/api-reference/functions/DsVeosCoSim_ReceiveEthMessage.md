# DsVeosCoSim_ReceiveEthMessage

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_ReceiveEthMessage](#dsveoscosim_receiveethmessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Receives an Ethernet message from the VEOS CoSim server.

> [!NOTE]
> If the [DsVeosCoSim_EthMessageReceivedCallback](DsVeosCoSim_EthMessageReceivedCallback.md) callback
> or the [DsVeosCoSim_EthMessageContainerReceivedCallback](DsVeosCoSim_EthMessageContainerReceivedCallback.md) callback is registered,
> Ethernet messages cannot be collected using the `DsVeosCoSim_ReceiveEthMessage` function.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ReceiveEthMessage(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_EthMessage* message
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_EthMessage](../structures/DsVeosCoSim_EthMessage.md)* message

A pointer to the Ethernet message.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
