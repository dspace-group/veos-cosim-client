# DsVeosCoSim_TerminateReasonToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_TerminateReasonToString](#dsveoscosim_terminatereasontostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Converts a terminate reason to a string.

## Syntax

```c
DSVEOSCOSIM_DECL const char* DsVeosCoSim_TerminateReasonToString(
    DsVeosCoSim_TerminateReason terminateReason
);
```

## Parameters

> [DsVeosCoSim_TerminateReason](../enumerations/DsVeosCoSim_TerminateReason.md) terminateReason

The terminate reason value to convert.

## Return values

> const char*

A string representation of the terminate reason value.
