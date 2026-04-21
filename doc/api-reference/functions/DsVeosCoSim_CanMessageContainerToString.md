# DsVeosCoSim_CanMessageContainerToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_CanMessageContainerToString](#dsveoscosim_canmessagecontainertostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a CAN message container as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_CanMessageContainerToString(
    const DsVeosCoSim_CanMessageContainer* messageContainer
);
```

## Parameters

> const [DsVeosCoSim_CanMessageContainer](../structures/DsVeosCoSim_CanMessageContainer.md)* messageContainer

The CAN message container to format.

## Return values

> std::string

A string representation of the CAN message container.
