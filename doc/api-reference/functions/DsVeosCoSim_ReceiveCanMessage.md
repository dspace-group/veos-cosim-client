# DsVeosCoSim_ReceiveCanMessage

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_ReceiveCanMessage](#dsveoscosim_receivecanmessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Receives a CAN message from the VEOS CoSim server.

> [!NOTE]
> If the [DsVeosCoSim_CanMessageReceivedCallback](DsVeosCoSim_CanMessageReceivedCallback.md) callback
> or the [DsVeosCoSim_CanMessageContainerReceivedCallback](DsVeosCoSim_CanMessageContainerReceivedCallback.md) callback is registered,
> CAN messages cannot be collected using the `DsVeosCoSim_ReceiveCanMessage` function.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ReceiveCanMessage(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_CanMessage* message
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_CanMessage](../structures/DsVeosCoSim_CanMessage.md)* message

A pointer to the received CAN message.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
