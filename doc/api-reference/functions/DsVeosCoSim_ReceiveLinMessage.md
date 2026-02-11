# DsVeosCoSim_ReceiveLinMessage

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_ReceiveLinMessage](#dsveoscosim_receivelinmessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Receives a LIN message from the VEOS CoSim server.

> [!NOTE]
> If the [DsVeosCoSim_LinMessageReceivedCallback](DsVeosCoSim_LinMessageReceivedCallback.md) callback
> or the [DsVeosCoSim_LinMessageContainerReceivedCallback](DsVeosCoSim_LinMessageContainerReceivedCallback.md) callback is registered,
> LIN messages cannot be collected using the `DsVeosCoSim_ReceiveLinMessage` function.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ReceiveLinMessage(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_LinMessage* message
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_LinMessage](../structures/DsVeosCoSim_LinMessage.md)* message

A pointer to the LIN message.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
