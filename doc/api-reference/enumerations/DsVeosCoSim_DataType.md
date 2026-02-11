# DsVeosCoSim_DataType

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_DataType](#dsveoscosim_datatype)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains valid data types for I/O signals.

## Syntax

```c
typedef enum DsVeosCoSim_DataType {
    DsVeosCoSim_DataType_Bool = 1,
    DsVeosCoSim_DataType_Int8,
    DsVeosCoSim_DataType_Int16,
    DsVeosCoSim_DataType_Int32,
    DsVeosCoSim_DataType_Int64,
    DsVeosCoSim_DataType_UInt8,
    DsVeosCoSim_DataType_UInt16,
    DsVeosCoSim_DataType_UInt32,
    DsVeosCoSim_DataType_UInt64,
    DsVeosCoSim_DataType_Float32,
    DsVeosCoSim_DataType_Float64
} DsVeosCoSim_DataType;
```

## Values

> DsVeosCoSim_DataType_Bool

The data type of the signal is Boolean. The C data type is `uint8_t`, where 0 equals false and any other value equals true.

> DsVeosCoSim_DataType_Int8

The data type of the signal is signed 8-bit integer (`int8_t`).

> DsVeosCoSim_DataType_Int16

The data type is signed 16-bit integer (`int16_t`).

> DsVeosCoSim_DataType_Int32

The data type is signed 32-bit integer (`int32_t`).

> DsVeosCoSim_DataType_Int64

The data type is signed 64-bit integer (`int64_t`).

> DsVeosCoSim_DataType_UInt8

The data type is unsigned 8-bit integer (`uint8_t`).

> DsVeosCoSim_DataType_UInt16

The data type is unsigned 16-bit integer (`uint16_t`).

> DsVeosCoSim_DataType_UInt32

The data type is unsigned 32-bit integer (`uint32_t`).

> DsVeosCoSim_DataType_UInt64

The data type is unsigned 64-bit integer (`uint64_t`).

> DsVeosCoSim_DataType_Float32

The data type is 32-bit float (`float`).

> DsVeosCoSim_DataType_Float64

The data type is 64-bit float (`double`).

## See Also

- [DsVeosCoSim_IoSignal](../structures/DsVeosCoSim_IoSignal.md)
