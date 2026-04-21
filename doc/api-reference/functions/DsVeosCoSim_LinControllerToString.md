# DsVeosCoSim_LinControllerToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_LinControllerToString](#dsveoscosim_lincontrollertostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a LIN controller as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_LinControllerToString(
    const DsVeosCoSim_LinController* controller
);
```

## Parameters

> const [DsVeosCoSim_LinController](../structures/DsVeosCoSim_LinController.md)* controller

The LIN controller to format.

## Return values

> std::string

A string representation of the LIN controller.
