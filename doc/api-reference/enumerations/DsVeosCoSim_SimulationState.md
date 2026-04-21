# DsVeosCoSim_SimulationState

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_SimulationState](#dsveoscosim_simulationstate)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains the possible simulation states.

## Syntax

```c
typedef enum DsVeosCoSim_SimulationState {
    DsVeosCoSim_SimulationState_Unloaded,
    DsVeosCoSim_SimulationState_Stopped,
    DsVeosCoSim_SimulationState_Running,
    DsVeosCoSim_SimulationState_Paused,
    DsVeosCoSim_SimulationState_Terminated
} DsVeosCoSim_SimulationState;
```

## Values

> DsVeosCoSim_SimulationState_Unloaded

The simulation is unloaded.

> DsVeosCoSim_SimulationState_Stopped

The simulation is stopped.

> DsVeosCoSim_SimulationState_Running

The simulation is running.

> DsVeosCoSim_SimulationState_Paused

The simulation is paused.

> DsVeosCoSim_SimulationState_Terminated

The simulation is terminated.

## See Also

- [DsVeosCoSim_GetSimulationState](../functions/DsVeosCoSim_GetSimulationState.md)
- [DsVeosCoSim_SimulationStateToString](../functions/DsVeosCoSim_SimulationStateToString.md)
