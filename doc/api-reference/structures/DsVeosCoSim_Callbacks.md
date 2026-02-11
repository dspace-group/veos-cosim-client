# DsVeosCoSim_Callbacks

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_Callbacks](#dsveoscosim_callbacks)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains the callbacks that can be called during the co-simulation.

## Syntax

```c
typedef struct DsVeosCoSim_Callbacks {
    DsVeosCoSim_SimulationCallback simulationStartedCallback;
    DsVeosCoSim_SimulationCallback simulationStoppedCallback;
    DsVeosCoSim_SimulationTerminatedCallback simulationTerminatedCallback;
    DsVeosCoSim_SimulationCallback simulationPausedCallback;
    DsVeosCoSim_SimulationCallback simulationContinuedCallback;
    DsVeosCoSim_SimulationCallback simulationBeginStepCallback;
    DsVeosCoSim_SimulationCallback simulationEndStepCallback;
    DsVeosCoSim_IncomingSignalChangedCallback incomingSignalChangedCallback;
    DsVeosCoSim_CanMessageReceivedCallback canMessageReceivedCallback;
    DsVeosCoSim_EthMessageReceivedCallback ethMessageReceivedCallback;
    DsVeosCoSim_LinMessageReceivedCallback linMessageReceivedCallback;
    void* userData;
    DsVeosCoSim_CanMessageContainerReceivedCallback canMessageContainerReceivedCallback;
    DsVeosCoSim_EthMessageContainerReceivedCallback ethMessageContainerReceivedCallback;
    DsVeosCoSim_LinMessageContainerReceivedCallback linMessageContainerReceivedCallback;
    DsVeosCoSim_FrMessageContainerReceivedCallback frMessageContainerReceivedCallback;
    DsVeosCoSim_FrMessageReceivedCallback frMessageReceivedCallback;
} DsVeosCoSim_Callbacks;
```

## Members

> [DsVeosCoSim_SimulationCallback](../function-pointers/DsVeosCoSim_SimulationCallback.md) simulationStartedCallback

Called when the simulation is started in VEOS.

> [DsVeosCoSim_SimulationCallback](../function-pointers/DsVeosCoSim_SimulationCallback.md) simulationStoppedCallback

Called when the simulation is stopped in VEOS.

> [DsVeosCoSim_SimulationTerminatedCallback](../function-pointers/DsVeosCoSim_SimulationTerminatedCallback.md) simulationTerminatedCallback

Called when the simulation is terminated in VEOS.

> [DsVeosCoSim_SimulationCallback](../function-pointers/DsVeosCoSim_SimulationCallback.md) simulationPausedCallback

Called when the simulation is paused in VEOS.

> [DsVeosCoSim_SimulationCallback](../function-pointers/DsVeosCoSim_SimulationCallback.md) simulationContinuedCallback

Called when the simulation is continued in VEOS.

> [DsVeosCoSim_SimulationCallback](../function-pointers/DsVeosCoSim_SimulationCallback.md) simulationBeginStepCallback

Called at the beginning of a simulation step.

> [DsVeosCoSim_SimulationCallback](../function-pointers/DsVeosCoSim_SimulationCallback.md) simulationEndStepCallback

Called at the end of a simulation step.

> [DsVeosCoSim_IncomingSignalChangedCallback](../function-pointers/DsVeosCoSim_IncomingSignalChangedCallback.md) incomingSignalChangedCallback

Called when an incoming signal value has changed.

> [DsVeosCoSim_CanMessageReceivedCallback](../function-pointers/DsVeosCoSim_CanMessageReceivedCallback.md) canMessageReceivedCallback

Called when a CAN message is received from VEOS.

> [DsVeosCoSim_EthMessageReceivedCallback](../function-pointers/DsVeosCoSim_EthMessageReceivedCallback.md) ethMessageReceivedCallback

Called when an Ethernet message is received from VEOS.

> [DsVeosCoSim_LinMessageReceivedCallback](../function-pointers/DsVeosCoSim_LinMessageReceivedCallback.md) linMessageReceivedCallback

Called when a LIN message is received from VEOS.

> `void*` userData

Arbitrary user data to be passed to every callback.

> [DsVeosCoSim_CanMessageContainerReceivedCallback](../function-pointers/DsVeosCoSim_CanMessageContainerReceivedCallback.md) canMessageContainerReceivedCallback

Called when a CAN message is received from VEOS.

> [DsVeosCoSim_LinMessageContainerReceivedCallback](../function-pointers/DsVeosCoSim_LinMessageContainerReceivedCallback.md) linMessageContainerReceivedCallback

Called when a LIN message is received from VEOS.

> [DsVeosCoSim_EthMessageContainerReceivedCallback](../function-pointers/DsVeosCoSim_EthMessageContainerReceivedCallback.md) ethMessageContainerReceivedCallback

Called when an Ethernet message is received from VEOS.

> [DsVeosCoSim_FrMessageContainerReceivedCallback](../function-pointers/DsVeosCoSim_FrMessageContainerReceivedCallback.md) frMessageContainerReceivedCallback

Called when a FlexRay message is received from VEOS.

> [DsVeosCoSim_FrMessageReceivedCallback](../function-pointers/DsVeosCoSim_FrMessageReceivedCallback.md) frMessageReceivedCallback

Called when a FlexRay message is received from VEOS.

## See Also

- [DsVeosCoSim_RunCallbackBasedCoSimulation](../functions/DsVeosCoSim_RunCallbackBasedCoSimulation.md)
- [DsVeosCoSim_StartPollingBasedCoSimulation](../functions/DsVeosCoSim_StartPollingBasedCoSimulation.md)
