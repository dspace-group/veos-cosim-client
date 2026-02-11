# DsVeosCoSim_LogCallback

[⬆️ Go to Function Pointers](function-pointers.md)

- [DsVeosCoSim\_LogCallback](#dsveoscosim_logcallback)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)
  - [See Also](#see-also)

## Description

Called each time the VEOS CoSim client receives a log message.

`DsVeosCoSim_LogCallback` can be registered with [DsVeosCoSim_SetLogCallback](DsVeosCoSim_SetLogCallback.md) function.

## Syntax

```c
typedef void (*DsVeosCoSim_LogCallback)(
    DsVeosCoSim_Severity severity,
    const char* logMessage
);
```

## Parameters

> [DsVeosCoSim_Severity](../enumerations/DsVeosCoSim_Severity.md) severity

The severity of the message.

> const char* logMessage

The log message content.

## Return values

This function has no return values.

## See Also

- [DsVeosCoSim_SetLogCallback](DsVeosCoSim_SetLogCallback.md)
