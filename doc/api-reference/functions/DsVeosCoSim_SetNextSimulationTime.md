# DsVeosCoSim_SetNextSimulationTime

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_SetNextSimulationTime](#dsveoscosim_setnextsimulationtime)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Sets the next simulation time for the given client handle.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_SetNextSimulationTime(
    DsVeosCoSim_Handle handle, 
    DsVeosCoSim_SimulationTime simulationTime
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The next simulation time.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
