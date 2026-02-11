# DsVeosCoSim_TransmitCanMessageContainer

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_TransmitCanMessageContainer](#dsveoscosim_transmitcanmessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Transmits a CAN message to the VEOS CoSim server.

The `DsVeosCoSim_TransmitCanMessageContainer` function can be called in any callback handler. The timestamp property of transmission bus messages will be ignored.

> [!NOTE]
> Currently, the DsVeosCoSim client cannot buffer more than 512 bus messages for each bus controller.
> If the buffer is full, the `DsVeosCoSim_TransmitCanMessageContainer` function returns [DsVeosCoSim_Result_Full](../enumerations/DsVeosCoSim_Result.md).

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_TransmitCanMessageContainer(
    DsVeosCoSim_Handle handle,
    const DsVeosCoSim_CanMessageContainer* messageContainer
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> const [DsVeosCoSim_CanMessageContainer](../structures/DsVeosCoSim_CanMessageContainer.md)* messageContainer

A pointer to a CAN message container to be transmitted.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
