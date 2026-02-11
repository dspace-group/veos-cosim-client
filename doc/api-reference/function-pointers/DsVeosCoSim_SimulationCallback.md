# DsVeosCoSim_SimulationCallback

[⬆️ Go to Function Pointers](function-pointers.md)

- [DsVeosCoSim\_SimulationCallback](#dsveoscosim_simulationcallback)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

A generic simulation callback function pointer.

## Syntax

```c
typedef void (*DsVeosCoSim_SimulationCallback)(
    DsVeosCoSim_SimulationTime simulationTime, 
    void* userData
);
```

## Parameters

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The current simulation time.

> void* userData

The user data passed to the co-simulation function via the [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md) structure. Can be `NULL`.

## Return values

This function has no return values.

## See Also

- [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md)
