# DsVeosCoSim_GetConnectionState

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetConnectionState](#dsveoscosim_getconnectionstate)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets the connection state for a given client handle.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetConnectionState(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_ConnectionState* connectionState
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_ConnectionState](../enumerations/DsVeosCoSim_ConnectionState.md)* connectionState

The connection state as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
