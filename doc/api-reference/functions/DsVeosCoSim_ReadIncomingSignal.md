# DsVeosCoSim_ReadIncomingSignal

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_ReadIncomingSignal](#dsveoscosim_readincomingsignal)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Reads a value from an incoming signal of the VEOS CoSim server identified by the given handle.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ReadIncomingSignal(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_IoSignalId incomingSignalId,
    uint32_t* length,
    void* value
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_IoSignalId](../simple-types/DsVeosCoSim_IoSignalId.md) incomingSignalId

The ID of the incoming signal.

> uint32_t* length

The length of the incoming signal value.

> void* value

The value of the incoming signal.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
