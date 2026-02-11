# DsVeosCoSim_CanMessage

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_CanMessage](#dsveoscosim_canmessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about a CAN message.

## Syntax

```c
typedef struct DsVeosCoSim_CanMessage {
    DsVeosCoSim_SimulationTime timestamp;
    DsVeosCoSim_BusControllerId controllerId;
    uint32_t id;
    DsVeosCoSim_CanMessageFlags flags;
    uint32_t length;
    const uint8_t* data;
} DsVeosCoSim_CanMessage;
```

## Members

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) timestamp

Contains the virtual simulation time at which the CAN message was received. Only for received messages.

> [DsVeosCoSim_BusControllerId](../simple-types/DsVeosCoSim_BusControllerId.md) controllerId

The ID of the CAN controller over which the CAN message was sent.

> uint32_t id

The CAN message ID.

> [DsVeosCoSim_CanMessageFlags](../enumerations/DsVeosCoSim_CanMessageFlags.md) flags

The flags of the CAN message.

> uint32_t length

The length of the CAN message payload in bytes.

> const uint8_t* data

The CAN message payload.

## See Also

- [DsVeosCoSim_CanMessageReceivedCallback](../functions/DsVeosCoSim_CanMessageReceivedCallback.md)
- [DsVeosCoSim_ReceiveCanMessage](../functions/DsVeosCoSim_ReceiveCanMessage.md)
- [DsVeosCoSim_TransmitCanMessage](../functions/DsVeosCoSim_TransmitCanMessage.md)
