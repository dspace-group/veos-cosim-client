# DsVeosCoSim_CanControllerToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_CanControllerToString](#dsveoscosim_cancontrollertostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a CAN controller as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_CanControllerToString(
    const DsVeosCoSim_CanController* controller
);
```

## Parameters

> const [DsVeosCoSim_CanController](../structures/DsVeosCoSim_CanController.md)* controller

The CAN controller to format.

## Return values

> std::string

A string representation of the CAN controller.
