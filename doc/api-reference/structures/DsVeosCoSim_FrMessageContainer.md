# DsVeosCoSim_FrMessageContainer

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_FrMessageContainer](#dsveoscosim_frmessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about a FlexRay message container.

## Syntax

```c
typedef struct DsVeosCoSim_FrMessageContainer {
    DsVeosCoSim_SimulationTime timestamp;
    DsVeosCoSim_BusControllerId controllerId;
    uint32_t reserved;
    uint32_t id;
    DsVeosCoSim_FrMessageFlags flags;
    uint32_t length;
    uint8_t data[DSVEOSCOSIM_FLEXRAY_MESSAGE_MAX_LENGTH];
} DsVeosCoSim_FrMessageContainer;
```

## Members

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) timestamp

Contains the virtual simulation time at which the FlexRay message was received. Only for received messages.

> [DsVeosCoSim_BusControllerId](../simple-types/DsVeosCoSim_BusControllerId.md) controllerId

The ID of the FlexRay controller over which the FlexRay message was sent.

> uint32_t reserved

Reserved for future use.

> uint32_t id

The FlexRay message ID.

> [DsVeosCoSim_FrMessageFlags](../enumerations/DsVeosCoSim_FrMessageFlags.md) flags

The flags of the FlexRay message.

> uint32_t length

The length of the FlexRay message payload in bytes.

> uint8_t data[[DSVEOSCOSIM_FLEXRAY_MESSAGE_MAX_LENGTH](../macros/DSVEOSCOSIM_FLEXRAY_MESSAGE_MAX_LENGTH.md)]

The FlexRay message payload.

## See Also

- [DsVeosCoSim_FrMessageContainerReceivedCallback](../functions/DsVeosCoSim_FrMessageContainerReceivedCallback.md)
- [DsVeosCoSim_ReceiveFrMessageContainer](../functions/DsVeosCoSim_ReceiveFrMessageContainer.md)
- [DsVeosCoSim_TransmitFrMessageContainer](../functions/DsVeosCoSim_TransmitFrMessageContainer.md)
