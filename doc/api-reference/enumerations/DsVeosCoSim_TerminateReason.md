# DsVeosCoSim_TerminateReason

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_TerminateReason](#dsveoscosim_terminatereason)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains the possible reasons for a termination of the co-simulation.

## Syntax

```c
typedef enum DsVeosCoSim_TerminateReason {
    DsVeosCoSim_TerminateReason_Finished,
    DsVeosCoSim_TerminateReason_Error,
} DsVeosCoSim_TerminateReason;
```

## Values

> DsVeosCoSim_TerminateReason_Finished

Indicates that the co-simulation was terminated because it finished successfully.

> DsVeosCoSim_TerminateReason_Error

Indicates that the co-simulation was terminated because of an error.

## See Also

- [DsVeosCoSim_SimulationTerminatedCallback](../function-pointers/DsVeosCoSim_SimulationTerminatedCallback.md)
