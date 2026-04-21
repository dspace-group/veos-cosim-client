# DsVeosCoSim_CanMessageFlagsToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_CanMessageFlagsToString](#dsveoscosim_canmessageflagstostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats CAN message flags as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_CanMessageFlagsToString(
    DsVeosCoSim_CanMessageFlags flags
);
```

## Parameters

> [DsVeosCoSim_CanMessageFlags](../enumerations/DsVeosCoSim_CanMessageFlags.md) flags

The CAN message flags to format.

## Return values

> std::string

A string representation of the CAN message flags.
