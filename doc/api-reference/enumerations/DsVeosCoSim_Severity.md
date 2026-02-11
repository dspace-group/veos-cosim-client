# DsVeosCoSim_Severity

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_Severity](#dsveoscosim_severity)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains all possible severity levels of a log message in descending order of severity.

## Syntax

```c
typedef enum DsVeosCoSim_Severity {
    DsVeosCoSim_Severity_Error
    DsVeosCoSim_Severity_Warning,
    DsVeosCoSim_Severity_Info,
    DsVeosCoSim_Severity_Trace,
} DsVeosCoSim_Severity;
```

## Values

> DsVeosCoSim_Severity_Error

The severity level is error.

> DsVeosCoSim_Severity_Warning

The severity level is warning.

> DsVeosCoSim_Severity_Info

The severity level is information.

> DsVeosCoSim_Severity_Trace

The severity level is trace.

## See Also

- [DsVeosCoSim_LogCallback](../function-pointers/DsVeosCoSim_LogCallback.md)
