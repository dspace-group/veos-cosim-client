# DsVeosCoSim_FrControllerToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_FrControllerToString](#dsveoscosim_frcontrollertostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a FlexRay controller as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_FrControllerToString(
    const DsVeosCoSim_FrController* controller
);
```

## Parameters

> const [DsVeosCoSim_FrController](../structures/DsVeosCoSim_FrController.md)* controller

The FlexRay controller to format.

## Return values

> std::string

A string representation of the FlexRay controller.
