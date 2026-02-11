# DsVeosCoSim_Connect

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_Connect](#dsveoscosim_connect)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Connects the VEOS CoSim client to the VEOS CoSim server.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_Connect(
    DsVeosCoSim_Handle handle,
    DsVeosCoSim_ConnectConfig connectConfig
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> [DsVeosCoSim_ConnectConfig](../structures/DsVeosCoSim_ConnectConfig.md) connectConfig

The data used for connecting to the VEOS CoSim server.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
