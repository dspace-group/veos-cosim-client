# DsVeosCoSim_LinControllerType

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_LinControllerType](#dsveoscosim_lincontrollertype)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains the LIN controller type.

## Syntax

```c
typedef enum DsVeosCoSim_LinControllerType {
    DsVeosCoSim_LinControllerType_Responder = 1,
    DsVeosCoSim_LinControllerType_Commander,
} DsVeosCoSim_LinControllerType;
```

## Values

> DsVeosCoSim_LinControllerType_Responder

Indicates that the LIN controller is a responder, i.e., slave.

> DsVeosCoSim_LinControllerType_Commander

Indicates that the LIN controller is a commander, i.e,. master.

## See Also

- [DsVeosCoSim_LinController](../structures/DsVeosCoSim_LinController.md)
