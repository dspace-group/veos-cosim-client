# DsVeosCoSim_ReceiveLinMessageContainer

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_ReceiveLinMessageContainer](#dsveoscosim_receivelinmessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Receives a LIN message from the VEOS CoSim server.

> [!NOTE]
> If the [DsVeosCoSim_LinMessageReceivedCallback](DsVeosCoSim_LinMessageReceivedCallback.md) callback
> or the [DsVeosCoSim_LinMessageContainerReceivedCallback](DsVeosCoSim_LinMessageContainerReceivedCallback.md) callback is registered,
> LIN messages cannot be collected using the `DsVeosCoSim_ReceiveLinMessageContainer` function.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ReceiveLinMessageContainer(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_LinMessageContainer* messageContainer
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_LinMessageContainer](../structures/DsVeosCoSim_LinMessageContainer.md)* messageContainer

A pointer to the LIN message container.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
