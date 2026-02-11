# DsVeosCoSim_GetIncomingSignals

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetIncomingSignals](#dsveoscosim_getincomingsignals)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets all available incoming signals.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetIncomingSignals(
    DsVeosCoSim_Handle handle,
    uint32_t* incomingSignalsCount,
    const DsVeosCoSim_IoSignal** incomingSignals
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> uint32_t* incomingSignalsCount

A pointer to the counter of incoming signals.

> const [DsVeosCoSim_IoSignal](../structures/DsVeosCoSim_IoSignal.md)** incomingSignals

A pointer to the array of incoming signals.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
