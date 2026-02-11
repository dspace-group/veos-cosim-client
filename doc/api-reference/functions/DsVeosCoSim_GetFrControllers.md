# DsVeosCoSim_GetFrControllers

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetFrControllers](#dsveoscosim_getfrcontrollers)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets all available FlexRay controllers in the co-simulation.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetFrControllers(
    DsVeosCoSim_Handle handle,
    uint32_t* frControllersCount,
    const DsVeosCoSim_FrController** frControllers
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> uint32_t* frControllersCount

A pointer to the counter of FlexRay controllers.

> const [DsVeosCoSim_FrController](../structures/DsVeosCoSim_FrController.md)** frControllers

A pointer to the array of FlexRay controllers.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
