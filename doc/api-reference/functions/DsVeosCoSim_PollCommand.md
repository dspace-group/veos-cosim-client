# DsVeosCoSim_PollCommand

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_PollCommand](#dsveoscosim_pollcommand)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Polls the simulator for a command.

`DsVeosCoSim_PollCommand` is only available if a polling based co-simulation was started with [DsVeosCoSim_StartPollingBasedCoSimulation](DsVeosCoSim_StartPollingBasedCoSimulation.md).

After `DsVeosCoSim_PollCommand` returns with a new command, the command must be finished with [DsVeosCoSim_FinishCommand](DsVeosCoSim_FinishCommand.md).

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_PollCommand(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_SimulationTime* simulationTime,
    DsVeosCoSim_Command* command
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md)* simulationTime

The current simulation time.

> [DsVeosCoSim_Command](../enumerations/DsVeosCoSim_Command.md)* command

The received command.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
