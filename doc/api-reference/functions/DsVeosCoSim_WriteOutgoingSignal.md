# DsVeosCoSim_WriteOutgoingSignal

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_WriteOutgoingSignal](#dsveoscosim_writeoutgoingsignal)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Writes a value to an outgoing signal of the VEOS CoSim server identified by the given handle.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_WriteOutgoingSignal(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_IoSignalId outgoingSignalId,
    uint32_t length,
    const void* value
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_IoSignalId](../simple-types/DsVeosCoSim_IoSignalId.md) outgoingSignalId

The ID of the outgoing signal.

> uint32_t length

The length of the value to write.

> const void* value

The value to write.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
