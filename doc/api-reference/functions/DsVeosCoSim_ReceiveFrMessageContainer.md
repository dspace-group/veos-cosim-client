# DsVeosCoSim_ReceiveFrMessageContainer

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_ReceiveFrMessageContainer](#dsveoscosim_receivefrmessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Receives a FlexRay message from the VEOS CoSim server.

> [!NOTE]
> If the [DsVeosCoSim_FrMessageReceivedCallback](DsVeosCoSim_FrMessageReceivedCallback.md) callback
> or the [DsVeosCoSim_FrMessageContainerReceivedCallback](DsVeosCoSim_FrMessageContainerReceivedCallback.md) callback is registered,
> FlexRay messages cannot be collected using the `DsVeosCoSim_ReceiveFrMessageContainer` function.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ReceiveFrMessageContainer(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_FrMessageContainer* messageContainer
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_FrMessageContainer](../structures/DsVeosCoSim_FrMessageContainer.md)* messageContainer

A pointer to the FlexRay message container.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
