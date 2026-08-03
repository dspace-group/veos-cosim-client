# DsVeosCoSim_GetRoundTripTime

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetRoundTripTime](#dsveoscosim_getroundtriptime)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets the round trip time.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetRoundTripTime(
    DsVeosCoSim_Handle handle,
    int64_t* roundTripTimeInNanoseconds
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> int64_t* roundTripTimeInNanoseconds

The round trip time in nanoseconds as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
