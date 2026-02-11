# DsVeosCoSim_TransmitCanMessage

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_TransmitCanMessage](#dsveoscosim_transmitcanmessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Transmits a CAN message to the VEOS CoSim server.

The `DsVeosCoSim_TransmitCanMessage` function can be called in any callback handler. The timestamp property of transmission bus messages will be ignored.

> [!NOTE]
> Currently, the DsVeosCoSim client cannot buffer more than 512 bus messages for each bus controller.
> If the buffer is full, the `DsVeosCoSim_TransmitCanMessage` function returns [DsVeosCoSim_Result_Full](../enumerations/DsVeosCoSim_Result.md).

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_TransmitCanMessage(
    DsVeosCoSim_Handle handle,
    const DsVeosCoSim_CanMessage* message
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> const [DsVeosCoSim_CanMessage](../structures/DsVeosCoSim_CanMessage.md)* message

A pointer to a CAN message to be transmitted.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
