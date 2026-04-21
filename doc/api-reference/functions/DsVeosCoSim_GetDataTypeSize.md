# DsVeosCoSim_GetDataTypeSize

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_GetDataTypeSize](#dsveoscosim_getdatatypesize)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Gets the size in bytes for a given data type.

## Syntax

```c
DSVEOSCOSIM_DECL size_t DsVeosCoSim_GetDataTypeSize(
    DsVeosCoSim_DataType dataType
);
```

## Parameters

> [DsVeosCoSim_DataType](../enumerations/DsVeosCoSim_DataType.md) dataType

The data type for which the size is requested.

## Return values

> size_t

The size of the data type in bytes.
