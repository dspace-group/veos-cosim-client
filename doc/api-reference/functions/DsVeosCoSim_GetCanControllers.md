# DsVeosCoSim_GetCanControllers

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetCanControllers](#dsveoscosim_getcancontrollers)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets all available CAN controllers in the co-simulation.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetCanControllers(
    DsVeosCoSim_Handle handle,
    uint32_t* canControllersCount,
    const DsVeosCoSim_CanController** canControllers
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> uint32_t* canControllersCount

A pointer to the counter of CAN controllers.

> const [DsVeosCoSim_CanController](../structures/DsVeosCoSim_CanController.md)** canControllers

A pointer to the array of CAN controllers.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
