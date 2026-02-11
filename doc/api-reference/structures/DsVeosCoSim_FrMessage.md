# DsVeosCoSim_FrMessage

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_FrMessage](#dsveoscosim_frmessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about a FlexRay message.

## Syntax

```c
typedef struct DsVeosCoSim_FrMessage {
    DsVeosCoSim_SimulationTime timestamp;
    DsVeosCoSim_BusControllerId controllerId;
    uint32_t id;
    DsVeosCoSim_FrMessageFlags flags;
    uint32_t length;
    const uint8_t* data;
} DsVeosCoSim_FrMessage;
```

## Members

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) timestamp

Contains the virtual simulation time at which the FlexRay message was received. Only for received messages.

> [DsVeosCoSim_BusControllerId](../simple-types/DsVeosCoSim_BusControllerId.md) controllerId

The ID of the FlexRay controller over which the FlexRay message was sent.

> uint32_t id

The FlexRay message ID.

> [DsVeosCoSim_FrMessageFlags](../enumerations/DsVeosCoSim_FrMessageFlags.md) flags

The flags of the FlexRay message.

> uint32_t length

The length of the FlexRay message payload in bytes.

> const uint8_t* data

The FlexRay message payload.

## See Also

- [DsVeosCoSim_FrMessageReceivedCallback](../functions/DsVeosCoSim_FrMessageReceivedCallback.md)
- [DsVeosCoSim_ReceiveFrMessage](../functions/DsVeosCoSim_ReceiveFrMessage.md)
- [DsVeosCoSim_TransmitFrMessage](../functions/DsVeosCoSim_TransmitFrMessage.md)
