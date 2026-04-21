# DsVeosCoSim_GetCurrentSimulationTime

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetCurrentSimulationTime](#dsveoscosim_getcurrentsimulationtime)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets the current simulation time.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetCurrentSimulationTime(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_SimulationTime* simulationTime
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md)* simulationTime

The current simulation time as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
