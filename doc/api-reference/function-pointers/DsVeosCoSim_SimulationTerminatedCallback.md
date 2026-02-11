# DsVeosCoSim_SimulationTerminatedCallback

[⬆️ Go to Function Pointers](function-pointers.md)

- [DsVeosCoSim\_SimulationTerminatedCallback](#dsveoscosim_simulationterminatedcallback)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

Called when a simulation is terminated.

## Syntax

```c
typedef void (*DsVeosCoSim_SimulationTerminatedCallback)(
    DsVeosCoSim_SimulationTime simulationTime,
    DsVeosCoSim_TerminateReason reason,
    void* userData
);
```

## Parameters

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The current simulation time.

> [DsVeosCoSim_TerminateReason](../enumerations/DsVeosCoSim_TerminateReason.md) reason

The reason for the termination.

> void* userData

The user data passed to the co-simulation function via the [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md) structure. Can be `NULL`.

## Return values

This function has no return values.

## See Also

- [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md)
