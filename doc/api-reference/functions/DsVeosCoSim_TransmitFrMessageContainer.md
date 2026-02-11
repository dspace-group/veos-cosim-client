# DsVeosCoSim_TransmitFrMessageContainer

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_TransmitFrMessageContainer](#dsveoscosim_transmitfrmessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Transmits a FlexRay message to the VEOS CoSim server.

The `DsVeosCoSim_TransmitFrMessageContainer` function can be called in any callback handler. The timestamp property of transmitted bus messages will be ignored.

> [!NOTE]
> Currently, the VEOS CoSim client cannot buffer more than `512` bus messages for each bus controller.
> If the buffer is full, the `DsVeosCoSim_TransmitFrMessageContainer` function returns [DsVeosCoSim_Result_Full](../enumerations/DsVeosCoSim_Result.md).

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_TransmitFrMessageContainer(
    DsVeosCoSim_Handle handle,
    const DsVeosCoSim_FrMessageContainer* messageContainer
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> const [DsVeosCoSim_FrMessageContainer](../structures/DsVeosCoSim_FrMessageContainer.md)* messageContainer

A pointer to the FlexRay message container to be transmitted.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
