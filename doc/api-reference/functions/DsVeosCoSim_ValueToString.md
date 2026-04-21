# DsVeosCoSim_ValueToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_ValueToString](#dsveoscosim_valuetostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a typed value buffer as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_ValueToString(
    DsVeosCoSim_DataType dataType,
    uint32_t length,
    const void* value
);
```

## Parameters

> [DsVeosCoSim_DataType](../enumerations/DsVeosCoSim_DataType.md) dataType

The data type of the value buffer.

> uint32_t length

The length of the value buffer.

> const void* value

The value buffer to format.

## Return values

> std::string

A string representation of the value buffer.
