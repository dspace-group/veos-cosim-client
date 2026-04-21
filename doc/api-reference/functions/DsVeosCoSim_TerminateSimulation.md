# DsVeosCoSim_TerminateSimulation

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_TerminateSimulation](#dsveoscosim_terminatesimulation)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Terminates the simulation.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_TerminateSimulation(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_TerminateReason terminateReason
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_TerminateReason](../enumerations/DsVeosCoSim_TerminateReason.md) terminateReason

The reason for the termination.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
