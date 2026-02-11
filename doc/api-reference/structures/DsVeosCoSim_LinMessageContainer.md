# DsVeosCoSim_LinMessageContainer

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_LinMessageContainer](#dsveoscosim_linmessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about a LIN message container.

## Syntax

```c
typedef struct DsVeosCoSim_LinMessageContainer {
    DsVeosCoSim_SimulationTime timestamp;
    DsVeosCoSim_BusControllerId controllerId;
    uint32_t reserved;
    uint32_t id;
    DsVeosCoSim_LinMessageFlags flags;
    uint32_t length;
    uint8_t data[DSVEOSCOSIM_LIN_MESSAGE_MAX_LENGTH];
} DsVeosCoSim_LinMessageContainer;
```

## Members

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) timestamp

Contains the virtual simulation time at which the LIN message was received. Only for received messages.

> [DsVeosCoSim_BusControllerId](../simple-types/DsVeosCoSim_BusControllerId.md) controllerId

The ID of the LIN controller over which the LIN message was sent.

> uint32_t reserved

Reserved for future use.

> uint32_t id

The LIN message ID.

> [DsVeosCoSim_LinMessageFlags](../enumerations/DsVeosCoSim_LinMessageFlags.md) flags

The flags of the LIN message.

> uint32_t length

The length of the LIN message payload in bytes.

> uint8_t data[[DSVEOSCOSIM_LIN_MESSAGE_MAX_LENGTH](../macros/DSVEOSCOSIM_LIN_MESSAGE_MAX_LENGTH.md)]

The LIN message payload.

## See Also

- [DsVeosCoSim_LinMessageContainerReceivedCallback](../functions/DsVeosCoSim_LinMessageContainerReceivedCallback.md)
- [DsVeosCoSim_ReceiveLinMessageContainer](../functions/DsVeosCoSim_ReceiveLinMessageContainer.md)
- [DsVeosCoSim_TransmitLinMessageContainer](../functions/DsVeosCoSim_TransmitLinMessageContainer.md)
