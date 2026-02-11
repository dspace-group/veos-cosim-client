# DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS

> [⬆️ Go to Macros](macros.md)

- [DSVEOSCOSIM\_SIMULATION\_TIME\_TO\_SECONDS](#dsveoscosim_simulation_time_to_seconds)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [See Also](#see-also)

## Description

Converts the given simulation time to a double in seconds.

## Syntax

```c
#define DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) ((double)(simulationTime) / DSVEOSCOSIM_SIMULATION_TIME_RESOLUTION_PER_SECOND)
```

## Parameters

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The simulation time to convert.

## See Also

- [DSVEOSCOSIM_SIMULATION_TIME_RESOLUTION_PER_SECOND](DSVEOSCOSIM_SIMULATION_TIME_RESOLUTION_PER_SECOND.md)
