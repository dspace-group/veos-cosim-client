# DsVeosCoSim_PollCommand2

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_PollCommand2](#dsveoscosim_pollcommand2)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

Polls the simulator for a command and waits up to the specified timeout.

`DsVeosCoSim_PollCommand2` is only available if a polling based co-simulation was started with [DsVeosCoSim_StartPollingBasedCoSimulation](DsVeosCoSim_StartPollingBasedCoSimulation.md).

Use `timeoutInMilliseconds` to bound how long the call waits for the next command. A value of `0` performs a non-blocking poll.

After `DsVeosCoSim_PollCommand2` returns with a new command, the command must be finished with [DsVeosCoSim_FinishCommand](DsVeosCoSim_FinishCommand.md).

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_PollCommand2(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_SimulationTime* simulationTime,
    DsVeosCoSim_Command* command,
    uint32_t timeoutInMilliseconds
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md)* simulationTime

The current simulation time.

> [DsVeosCoSim_Command](../enumerations/DsVeosCoSim_Command.md)* command

The received command.

> uint32_t timeoutInMilliseconds

The maximum time to wait in milliseconds. Use `0` for non-blocking.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
Returns [DsVeosCoSim_Result_Timeout](../enumerations/DsVeosCoSim_Result.md) if no command arrives before the timeout expires.

## See Also

- [DsVeosCoSim_StartPollingBasedCoSimulation](DsVeosCoSim_StartPollingBasedCoSimulation.md)
- [DsVeosCoSim_PollCommand](DsVeosCoSim_PollCommand.md)
- [DsVeosCoSim_FinishCommand](DsVeosCoSim_FinishCommand.md)
