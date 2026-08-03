# DsVeosCoSim_CanMessageToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_CanMessageToString](#dsveoscosim_canmessagetostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a CAN message as a string.

This function follows a two-call pattern. Call it once with `buffer` set to `NULL` to obtain the required buffer size (including the terminating null) in `bufferSize`, then call it again with a buffer of at least that size.

## Syntax

```cpp
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_CanMessageToString(
    const DsVeosCoSim_CanMessage* message,
    char* buffer,
    uint32_t* bufferSize
);
```

## Parameters

> const [DsVeosCoSim_CanMessage](../structures/DsVeosCoSim_CanMessage.md)* message

The CAN message to format.

> char* buffer

The destination buffer, or `NULL` to query the required buffer size.

> uint32_t* bufferSize

On input, the size of `buffer`. On output, the required size including the terminating null.

## Return values

> [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md)

`DsVeosCoSim_Result_Ok` on success. `DsVeosCoSim_Result_Full` if `buffer` is `NULL` or too small; in that case the required size (including the terminating null) is written to `bufferSize`. `DsVeosCoSim_Result_InvalidArgument` if a required argument is `NULL`.
