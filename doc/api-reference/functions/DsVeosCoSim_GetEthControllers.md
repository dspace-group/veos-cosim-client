# DsVeosCoSim_GetEthControllers

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetEthControllers](#dsveoscosim_getethcontrollers)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets all available Ethernet controllers in the co-simulation.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetEthControllers(
    DsVeosCoSim_Handle handle,
    uint32_t* ethControllersCount,
    const DsVeosCoSim_EthController** ethControllers
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> uint32_t* ethControllersCount

A pointer to the counter of Ethernet controllers.

> const [DsVeosCoSim_EthController](../structures/DsVeosCoSim_EthController.md)** ethControllers

A pointer to the array of Ethernet controllers.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
