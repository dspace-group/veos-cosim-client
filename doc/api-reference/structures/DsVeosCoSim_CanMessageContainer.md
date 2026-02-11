# DsVeosCoSim_CanMessageContainer

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_CanMessageContainer](#dsveoscosim_canmessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about a CAN message container.

## Syntax

```c
typedef struct DsVeosCoSim_CanMessageContainer {
    DsVeosCoSim_SimulationTime timestamp;
    DsVeosCoSim_BusControllerId controllerId;
    uint32_t reserved;
    uint32_t id;
    DsVeosCoSim_CanMessageFlags flags;
    uint32_t length;
    uint8_t data[DSVEOSCOSIM_CAN_MESSAGE_MAX_LENGTH];
} DsVeosCoSim_CanMessageContainer;
```

## Members

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) timestamp

Contains the virtual simulation time at which the CAN message was received. Only for received messages.

> [DsVeosCoSim_BusControllerId](../simple-types/DsVeosCoSim_BusControllerId.md) controllerId

The ID of the CAN controller over which the CAN message was sent.

> uint32_t reserved

Reserved for future use.

> uint32_t id

The CAN message ID.

> [DsVeosCoSim_CanMessageFlags](../enumerations/DsVeosCoSim_CanMessageFlags.md) flags

The flags of the CAN message.

> uint32_t length

The length of the CAN message payload in bytes.

> uint8_t data[[DSVEOSCOSIM_CAN_MESSAGE_MAX_LENGTH](../macros/DSVEOSCOSIM_CAN_MESSAGE_MAX_LENGTH.md)]

The CAN message payload.

## See Also

- [DsVeosCoSim_CanMessageContainerReceivedCallback](../functions/DsVeosCoSim_CanMessageContainerReceivedCallback.md)
- [DsVeosCoSim_ReceiveCanMessageContainer](../functions/DsVeosCoSim_ReceiveCanMessageContainer.md)
- [DsVeosCoSim_TransmitCanMessageContainer](../functions/DsVeosCoSim_TransmitCanMessageContainer.md)
