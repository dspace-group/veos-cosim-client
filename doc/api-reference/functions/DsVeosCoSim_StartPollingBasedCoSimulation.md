# DsVeosCoSim_StartPollingBasedCoSimulation

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_StartPollingBasedCoSimulation](#dsveoscosim_startpollingbasedcosimulation)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Starts a co-simulation in non-blocking mode.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_StartPollingBasedCoSimulation(
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
