# DsVeosCoSim_RunCallbackBasedCoSimulation

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_RunCallbackBasedCoSimulation](#dsveoscosim_runcallbackbasedcosimulation)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Starts a callback-based co-simulation.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_RunCallbackBasedCoSimulation(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_Callbacks callbacks
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md) callbacks

The callbacks to be set.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
