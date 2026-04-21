# DsVeosCoSim_EthControllerToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_EthControllerToString](#dsveoscosim_ethcontrollertostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats an Ethernet controller as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_EthControllerToString(
    const DsVeosCoSim_EthController* controller
);
```

## Parameters

> const [DsVeosCoSim_EthController](../structures/DsVeosCoSim_EthController.md)* controller

The Ethernet controller to format.

## Return values

> std::string

A string representation of the Ethernet controller.
