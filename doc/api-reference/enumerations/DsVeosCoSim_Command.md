# DsVeosCoSim_Command

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_Command](#dsveoscosim_command)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains commands for controlling the co-simulation in polling mode.

## Syntax

```c
typedef enum DsVeosCoSim_Command {
    DsVeosCoSim_Command_None,
    DsVeosCoSim_Command_Step,
    DsVeosCoSim_Command_Start,
    DsVeosCoSim_Command_Stop,
    DsVeosCoSim_Command_Terminate,
    DsVeosCoSim_Command_Pause,
    DsVeosCoSim_Command_Continue,
} DsVeosCoSim_Command;
```

## Values

> DsVeosCoSim_Command_None

No simulation command

> DsVeosCoSim_Command_Step

Advance the simulation by one step.

> DsVeosCoSim_Command_Start

Start the simulation.

> DsVeosCoSim_Command_Stop

Stop the simulation.

> DsVeosCoSim_Command_Terminate

Terminate the simulation.

> DsVeosCoSim_Command_Pause

Pause the simulation.

> DsVeosCoSim_Command_Continue

Continue the simulation.

## See Also

- [DsVeosCoSim_PollCommand](../functions/DsVeosCoSim_PollCommand.md)
