# DsVeosCoSim_LinMessageContainerToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_LinMessageContainerToString](#dsveoscosim_linmessagecontainertostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a LIN message container as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_LinMessageContainerToString(
    const DsVeosCoSim_LinMessageContainer* messageContainer
);
```

## Parameters

> const [DsVeosCoSim_LinMessageContainer](../structures/DsVeosCoSim_LinMessageContainer.md)* messageContainer

The LIN message container to format.

## Return values

> std::string

A string representation of the LIN message container.
