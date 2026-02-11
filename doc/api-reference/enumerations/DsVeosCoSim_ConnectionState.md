# DsVeosCoSim_ConnectionState

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_ConnectionState](#dsveoscosim_connectionstate)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains the possible connection states.

## Syntax

```c
typedef enum DsVeosCoSim_ConnectionState {
    DsVeosCoSim_ConnectionState_Disconnected,
    DsVeosCoSim_ConnectionState_Connected,
} DsVeosCoSim_ConnectionState;
```

## Values

> DsVeosCoSim_ConnectionState_Disconnected

Indicates that the VEOS CoSim client is disconnected from the server.

> DsVeosCoSim_ConnectionState_Connected

Indicates that the VEOS CoSim client is connected to the server.

## See Also

- [DsVeosCoSim_GetConnectionState](../functions/DsVeosCoSim_GetConnectionState.md)
