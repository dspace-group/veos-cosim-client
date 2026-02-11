# DsVeosCoSim_TransmitLinMessageContainer

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_TransmitLinMessageContainer](#dsveoscosim_transmitlinmessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Transmits a LIN message to the VEOS CoSim server.

The `DsVeosCoSim_TransmitLinMessageContainer` function can be called in any callback handler. The timestamp property of transmitted bus messages will be ignored.

> [!NOTE]
> Currently, the VEOS CoSim client cannot buffer more than `512` bus messages for each bus controller.
> If the buffer is full, the `DsVeosCoSim_TransmitLinMessageContainer` function returns [DsVeosCoSim_Result_Full](../enumerations/DsVeosCoSim_Result.md).

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_TransmitLinMessageContainer(
    DsVeosCoSim_Handle handle,
    const DsVeosCoSim_LinMessageContainer* messageContainer
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> const [DsVeosCoSim_LinMessageContainer](../structures/DsVeosCoSim_LinMessageContainer.md)* messageContainer

A pointer to the LIN message container to be transmitted.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
