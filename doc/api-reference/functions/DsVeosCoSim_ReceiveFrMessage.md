# DsVeosCoSim_ReceiveFrMessage

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_ReceiveFrMessage](#dsveoscosim_receivefrmessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Receives a FlexRay message from the VEOS CoSim server.

> [!NOTE]
> If the [DsVeosCoSim_FrMessageReceivedCallback](DsVeosCoSim_FrMessageReceivedCallback.md) callback
> or the [DsVeosCoSim_FrMessageContainerReceivedCallback](DsVeosCoSim_FrMessageContainerReceivedCallback.md) callback is registered,
> FlexRay messages cannot be collected using the `DsVeosCoSim_ReceiveFrMessage` function.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ReceiveFrMessage(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_FrMessage* message
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_FrMessage](../structures/DsVeosCoSim_FrMessage.md)* message

A pointer to the FlexRay message.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
