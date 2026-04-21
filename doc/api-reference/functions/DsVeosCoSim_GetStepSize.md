# DsVeosCoSim_GetStepSize

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetStepSize](#dsveoscosim_getstepsize)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets the step size of the VEOS CoSim server.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetStepSize(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_SimulationTime* stepSize
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md)* stepSize

The step size as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
