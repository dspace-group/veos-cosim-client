# DsVeosCoSim_GetSimulationState

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetSimulationState](#dsveoscosim_getsimulationstate)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets the current simulation state.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetSimulationState(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_SimulationState* simulationState
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_SimulationState](../enumerations/DsVeosCoSim_SimulationState.md)* simulationState

The simulation state as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
