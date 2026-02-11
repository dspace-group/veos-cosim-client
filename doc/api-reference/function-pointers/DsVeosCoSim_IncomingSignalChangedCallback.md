# DsVeosCoSim_IncomingSignalChangedCallback

[⬆️ Go to Function Pointers](function-pointers.md)

- [DsVeosCoSim\_IncomingSignalChangedCallback](#dsveoscosim_incomingsignalchangedcallback)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

Represents an incoming signal changed callback function pointer.

## Syntax

```c
typedef void (*DsVeosCoSim_IncomingSignalChangedCallback)(
    DsVeosCoSim_SimulationTime simulationTime,
    const DsVeosCoSim_IoSignal* ioSignal,
    uint32_t length,
    const void* value,
    void* userData
);
```

## Parameters

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) simulationTime

The current simulation time.

> const [DsVeosCoSim_IoSignal](../structures/DsVeosCoSim_IoSignal.md)* ioSignal

The ID of the I/O signal that changed its value.

> uint32_t length

The length of the changed data in element count.

> const void* value

A pointer to the current value of the I/O signal.

> void* userData

The user data passed to the co-simulation function via the [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md) structure. Can be `NULL`.

## Return values

This function has no return values.

## See Also

- [DsVeosCoSim_Callbacks](../structures/DsVeosCoSim_Callbacks.md)
