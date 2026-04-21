# DsVeosCoSim_EthMessageContainerToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_EthMessageContainerToString](#dsveoscosim_ethmessagecontainertostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats an Ethernet message container as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_EthMessageContainerToString(
    const DsVeosCoSim_EthMessageContainer* messageContainer
);
```

## Parameters

> const [DsVeosCoSim_EthMessageContainer](../structures/DsVeosCoSim_EthMessageContainer.md)* messageContainer

The Ethernet message container to format.

## Return values

> std::string

A string representation of the Ethernet message container.
