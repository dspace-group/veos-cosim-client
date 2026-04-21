# DsVeosCoSim_EthMessageFlagsToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_EthMessageFlagsToString](#dsveoscosim_ethmessageflagstostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats Ethernet message flags as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_EthMessageFlagsToString(
    DsVeosCoSim_EthMessageFlags flags
);
```

## Parameters

> [DsVeosCoSim_EthMessageFlags](../enumerations/DsVeosCoSim_EthMessageFlags.md) flags

The Ethernet message flags to format.

## Return values

> std::string

A string representation of the Ethernet message flags.
