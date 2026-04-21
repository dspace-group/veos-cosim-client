# DsVeosCoSim_FinishCommand

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_FinishCommand](#dsveoscosim_finishcommand)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

Finishes the current command in a polling-based simulation.

`DsVeosCoSim_FinishCommand` must be called after each successful call to [DsVeosCoSim_PollCommand](DsVeosCoSim_PollCommand.md) or [DsVeosCoSim_PollCommand2](DsVeosCoSim_PollCommand2.md) that returns a command.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_FinishCommand(
    DsVeosCoSim_Handle handle
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).

## See Also

- [DsVeosCoSim_StartPollingBasedCoSimulation](DsVeosCoSim_StartPollingBasedCoSimulation.md)
- [DsVeosCoSim_PollCommand](DsVeosCoSim_PollCommand.md)
- [DsVeosCoSim_PollCommand2](DsVeosCoSim_PollCommand2.md)
