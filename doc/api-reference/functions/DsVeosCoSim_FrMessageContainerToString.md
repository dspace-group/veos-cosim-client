# DsVeosCoSim_FrMessageContainerToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_FrMessageContainerToString](#dsveoscosim_frmessagecontainertostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a FlexRay message container as a string.

This function follows a two-call pattern. Call it once with `buffer` set to `NULL` to obtain the required buffer size (including the terminating null) in `bufferSize`, then call it again with a buffer of at least that size.

## Syntax

```cpp
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_FrMessageContainerToString(
    const DsVeosCoSim_FrMessageContainer* messageContainer,
    char* buffer,
    uint32_t* bufferSize
);
```

## Parameters

> const [DsVeosCoSim_FrMessageContainer](../structures/DsVeosCoSim_FrMessageContainer.md)* messageContainer

The FlexRay message container to format.

> char* buffer

The destination buffer, or `NULL` to query the required buffer size.

> uint32_t* bufferSize

On input, the size of `buffer`. On output, the required size including the terminating null.

## Return values

> [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md)

`DsVeosCoSim_Result_Ok` on success. `DsVeosCoSim_Result_Full` if `buffer` is `NULL` or too small; in that case the required size (including the terminating null) is written to `bufferSize`. `DsVeosCoSim_Result_InvalidArgument` if a required argument is `NULL`.
