# DsVeosCoSim_FrMessageContainerToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_FrMessageContainerToString](#dsveoscosim_frmessagecontainertostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a FlexRay message container as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_FrMessageContainerToString(
    const DsVeosCoSim_FrMessageContainer* messageContainer
);
```

## Parameters

> const [DsVeosCoSim_FrMessageContainer](../structures/DsVeosCoSim_FrMessageContainer.md)* messageContainer

The FlexRay message container to format.

## Return values

> std::string

A string representation of the FlexRay message container.
