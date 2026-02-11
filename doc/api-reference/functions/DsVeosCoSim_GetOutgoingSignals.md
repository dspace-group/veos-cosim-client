# DsVeosCoSim_GetOutgoingSignals

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetOutgoingSignals](#dsveoscosim_getoutgoingsignals)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets all available outgoing signals.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetOutgoingSignals(
    DsVeosCoSim_Handle handle,
    uint32_t* outgoingSignalsCount,
    const DsVeosCoSim_IoSignal** outgoingSignals
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> uint32_t* outgoingSignalsCount

A pointer to the counter of outgoing signals.

> const [DsVeosCoSim_IoSignal](../structures/DsVeosCoSim_IoSignal.md)** outgoingSignals

A pointer to the array of outgoing signals.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
