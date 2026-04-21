# DsVeosCoSim_FrMessageFlagsToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_FrMessageFlagsToString](#dsveoscosim_frmessageflagstostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats FlexRay message flags as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_FrMessageFlagsToString(
    DsVeosCoSim_FrMessageFlags flags
);
```

## Parameters

> [DsVeosCoSim_FrMessageFlags](../enumerations/DsVeosCoSim_FrMessageFlags.md) flags

The FlexRay message flags to format.

## Return values

> std::string

A string representation of the FlexRay message flags.
