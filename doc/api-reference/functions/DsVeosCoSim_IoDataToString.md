# DsVeosCoSim_IoDataToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_IoDataToString](#dsveoscosim_iodatatostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats I/O signal data as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_IoDataToString(
    const DsVeosCoSim_IoSignal* ioSignal,
    uint32_t length,
    const void* value
);
```

## Parameters

> const [DsVeosCoSim_IoSignal](../structures/DsVeosCoSim_IoSignal.md)* ioSignal

The signal metadata for the value buffer.

> uint32_t length

The length of the value buffer.

> const void* value

The value buffer to format.

## Return values

> std::string

A string representation of the I/O signal data.
