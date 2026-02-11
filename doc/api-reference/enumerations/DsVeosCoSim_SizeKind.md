# DsVeosCoSim_SizeKind

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_SizeKind](#dsveoscosim_sizekind)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains information on the I/O signal length type.

## Syntax

```c
typedef enum DsVeosCoSim_SizeKind {
    DsVeosCoSim_SizeKind_Fixed = 1,
    DsVeosCoSim_SizeKind_Variable,
} DsVeosCoSim_SizeKind;
```

## Values

> DsVeosCoSim_SizeKind_Fixed

Indicates that the I/O signal has a fixed length.

> DsVeosCoSim_SizeKind_Variable

Indicates that the I/O signal has a variable length in the range of 0 to max length.

## See Also

- [DsVeosCoSim_IoSignal](../structures/DsVeosCoSim_IoSignal.md)
