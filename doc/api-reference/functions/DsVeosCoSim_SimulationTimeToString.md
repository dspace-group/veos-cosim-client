# DsVeosCoSim_SimulationTimeToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_SimulationTimeToString](#dsveoscosim_simulationtimetostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a simulation time as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_SimulationTimeToString(
    DsVeosCoSim_SimulationTime simulationTime
);
```

## Parameters

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The simulation time to format.

## Return values

> std::string

A string representation of the simulation time.
