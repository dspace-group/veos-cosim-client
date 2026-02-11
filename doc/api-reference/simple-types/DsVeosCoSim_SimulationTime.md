# DsVeosCoSim_SimulationTime

> [⬆️ Go to Simple Types](simple-types.md)

- [DsVeosCoSim\_SimulationTime](#dsveoscosim_simulationtime)
  - [Description](#description)
  - [Syntax](#syntax)
  - [See Also](#see-also)

## Description

Represents the simulation time in nanoseconds.

Divide by [DSVEOSCOSIM_SIMULATION_TIME_RESOLUTION_PER_SECOND](../macros/DSVEOSCOSIM_SIMULATION_TIME_RESOLUTION_PER_SECOND.md) to get the simulation time in seconds.

## Syntax

```c
typedef int64_t DsVeosCoSim_SimulationTime;
```

## See Also

- [DSVEOSCOSIM_SIMULATION_TIME_RESOLUTION_PER_SECOND](../macros/DSVEOSCOSIM_SIMULATION_TIME_RESOLUTION_PER_SECOND.md)
- [DsVeosCoSim_CanMessage](../structures/DsVeosCoSim_CanMessage.md)
- [DsVeosCoSim_CanMessageContainer](../structures/DsVeosCoSim_CanMessageContainer.md)
- [DsVeosCoSim_EthMessage](../structures/DsVeosCoSim_EthMessage.md)
- [DsVeosCoSim_EthMessageContainer](../structures/DsVeosCoSim_EthMessageContainer.md)
- [DsVeosCoSim_LinMessage](../structures/DsVeosCoSim_LinMessage.md)
- [DsVeosCoSim_LinMessageContainer](../structures/DsVeosCoSim_LinMessageContainer.md)
- [DsVeosCoSim_FrMessage](../structures/DsVeosCoSim_FrMessage.md)
- [DsVeosCoSim_FrMessageContainer](../structures/DsVeosCoSim_FrMessageContainer.md)
- [DsVeosCoSim_SimulationCallback](../function-pointers/DsVeosCoSim_SimulationCallback.md)
- [DsVeosCoSim_SimulationTerminatedCallback](../function-pointers/DsVeosCoSim_SimulationTerminatedCallback.md)
- [DsVeosCoSim_IncomingSignalChangedCallback](../function-pointers/DsVeosCoSim_IncomingSignalChangedCallback.md)
- [DsVeosCoSim_CanMessageReceivedCallback](../function-pointers/DsVeosCoSim_CanMessageReceivedCallback.md)
- [DsVeosCoSim_CanMessageContainerReceivedCallback](../function-pointers/DsVeosCoSim_CanMessageContainerReceivedCallback.md)
- [DsVeosCoSim_EthMessageReceivedCallback](../function-pointers/DsVeosCoSim_EthMessageReceivedCallback.md)
- [DsVeosCoSim_EthMessageContainerReceivedCallback](../function-pointers/DsVeosCoSim_EthMessageContainerReceivedCallback.md)
- [DsVeosCoSim_LinMessageReceivedCallback](../function-pointers/DsVeosCoSim_LinMessageReceivedCallback.md)
- [DsVeosCoSim_LinMessageContainerReceivedCallback](../function-pointers/DsVeosCoSim_LinMessageContainerReceivedCallback.md)
- [DsVeosCoSim_FrMessageReceivedCallback](../function-pointers/DsVeosCoSim_FrMessageReceivedCallback.md)
- [DsVeosCoSim_FrMessageContainerReceivedCallback](../function-pointers/DsVeosCoSim_FrMessageContainerReceivedCallback.md)
- [DsVeosCoSim_PollCommand](../functions/DsVeosCoSim_PollCommand.md)
- [DsVeosCoSim_SetNextSimulationTime](../functions/DsVeosCoSim_SetNextSimulationTime.md)
- [DsVeosCoSim_GetStepSize](../functions/DsVeosCoSim_GetStepSize.md)
