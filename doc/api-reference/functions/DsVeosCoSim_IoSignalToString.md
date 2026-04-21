# DsVeosCoSim_IoSignalToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_IoSignalToString](#dsveoscosim_iosignaltostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats an I/O signal as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_IoSignalToString(
    const DsVeosCoSim_IoSignal* ioSignal
);
```

## Parameters

> const [DsVeosCoSim_IoSignal](../structures/DsVeosCoSim_IoSignal.md)* ioSignal

The I/O signal to format.

## Return values

> std::string

A string representation of the I/O signal.
