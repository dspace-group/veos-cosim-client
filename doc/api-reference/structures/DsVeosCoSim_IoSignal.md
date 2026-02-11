# DsVeosCoSim_IoSignal

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_IoSignal](#dsveoscosim_iosignal)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about an I/O signal.

## Syntax

```c
typedef struct DsVeosCoSim_IoSignal {
    DsVeosCoSim_IoSignalId id;
    uint32_t length;
    DsVeosCoSim_DataType dataType;
    DsVeosCoSim_SizeKind sizeKind;
    const char* name;
} DsVeosCoSim_IoSignal;
```

## Members

> [DsVeosCoSim_IoSignalId](../simple-types/DsVeosCoSim_IoSignalId.md) id

The unique identifier of the I/O signal.

> uint32_t length

The exact length of the I/O signal if it is of fixed length or the maximum length if it is of variable size.

> [DsVeosCoSim_DataType](../enumerations/DsVeosCoSim_DataType.md) dataType

The data type of the I/O signal.

> [DsVeosCoSim_SizeKind](../enumerations/DsVeosCoSim_SizeKind.md) sizeKind

The size kind of the the I/O signal.

> const char* name

The name of the I/O signal.

## See Also

- [DsVeosCoSim_IncomingSignalChangedCallback](../functions/DsVeosCoSim_IncomingSignalChangedCallback.md)
- [DsVeosCoSim_GetIncomingSignals](../functions/DsVeosCoSim_GetIncomingSignals.md)
- [DsVeosCoSim_GetOutgoingSignals](../functions/DsVeosCoSim_GetOutgoingSignals.md)
