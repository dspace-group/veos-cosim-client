# DsVeosCoSim_LinMessageFlagsToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_LinMessageFlagsToString](#dsveoscosim_linmessageflagstostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats LIN message flags as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_LinMessageFlagsToString(
    DsVeosCoSim_LinMessageFlags flags
);
```

## Parameters

> [DsVeosCoSim_LinMessageFlags](../enumerations/DsVeosCoSim_LinMessageFlags.md) flags

The LIN message flags to format.

## Return values

> std::string

A string representation of the LIN message flags.
