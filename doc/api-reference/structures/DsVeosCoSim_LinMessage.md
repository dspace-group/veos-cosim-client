# DsVeosCoSim_LinMessage

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_LinMessage](#dsveoscosim_linmessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about a LIN message.

## Syntax

```c
typedef struct DsVeosCoSim_LinMessage {
    DsVeosCoSim_SimulationTime timestamp;
    DsVeosCoSim_BusControllerId controllerId;
    uint32_t id;
    DsVeosCoSim_LinMessageFlags flags;
    uint32_t length;
    const uint8_t* data;
} DsVeosCoSim_LinMessage;
```

## Members

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) timestamp

Contains the virtual simulation time at which the LIN message was received. Only for received messages.

> [DsVeosCoSim_BusControllerId](../simple-types/DsVeosCoSim_BusControllerId.md) controllerId

The ID of the LIN controller over which the LIN message was sent.

> uint32_t id

The LIN message ID.

> [DsVeosCoSim_LinMessageFlags](../enumerations/DsVeosCoSim_LinMessageFlags.md) flags

The flags of the LIN message.

> uint32_t length

The length of the LIN message payload in bytes.

> const uint8_t* data

The LIN message payload.

## See Also

- [DsVeosCoSim_LinMessageReceivedCallback](../functions/DsVeosCoSim_LinMessageReceivedCallback.md)
- [DsVeosCoSim_ReceiveLinMessage](../functions/DsVeosCoSim_ReceiveLinMessage.md)
- [DsVeosCoSim_TransmitLinMessage](../functions/DsVeosCoSim_TransmitLinMessage.md)
