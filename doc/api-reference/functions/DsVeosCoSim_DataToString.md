# DsVeosCoSim_DataToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_DataToString](#dsveoscosim_datatostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats raw byte data as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_DataToString(
    const uint8_t* data,
    size_t dataLength,
    char separator = 0
);
```

## Parameters

> const uint8_t* data

The byte data to format.

> size_t dataLength

The number of bytes to format.

> char separator

The separator character used while formatting.

## Return values

> std::string

A string representation of the byte data.
