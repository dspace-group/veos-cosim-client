# DsVeosCoSim_GetLinControllers

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetLinControllers](#dsveoscosim_getlincontrollers)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets all available LIN controllers in the co-simulation.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetLinControllers(
    DsVeosCoSim_Handle handle,
    uint32_t* linControllersCount,
    const DsVeosCoSim_LinController** linControllers
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> uint32_t* linControllersCount

A pointer to the counter of LIN controllers.

> const [DsVeosCoSim_LinController](../structures/DsVeosCoSim_LinController.md)** linControllers

A pointer to the array of LIN controllers.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
