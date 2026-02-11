# DsVeosCoSim_Disconnect

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_Disconnect](#dsveoscosim_disconnect)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Disconnects the VEOS CoSim client from the VEOS CoSim server.

If there is no connection to the VEOS CoSim server, the `DsVeosCoSim_Disconnect` function does not do or return anything.

If the connection is closed during a running simulation, the simulation terminates with an error.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_Disconnect(
    DsVeosCoSim_Handle handle
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
