# DsVeosCoSim_EthMessage

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_EthMessage](#dsveoscosim_ethmessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about an Ethernet message.

## Syntax

```c
typedef struct DsVeosCoSim_EthMessage {
    DsVeosCoSim_SimulationTime timestamp;
    DsVeosCoSim_BusControllerId controllerId;
    uint32_t reserved;
    DsVeosCoSim_EthMessageFlags flags;
    uint32_t length;
    const uint8_t* data;
} DsVeosCoSim_EthMessage;
```

## Members

> [DsVeosCoSim_SimulationTime](../simple-types/DsVeosCoSim_SimulationTime.md) timestamp

Contains the virtual simulation time at which the Ethernet message was received. Only for received messages.

> [DsVeosCoSim_BusControllerId](../simple-types/DsVeosCoSim_BusControllerId.md) controllerId

The ID of the Ethernet controller over which the Ethernet message was sent.

> uint32_t reserved

Reserved for future use.

> [DsVeosCoSim_EthMessageFlags](../enumerations/DsVeosCoSim_EthMessageFlags.md) flags

The flags of the Ethernet message.

> uint32_t length

The length of the Ethernet message payload in bytes.

> const uint8_t* data

The Ethernet message payload.

## See Also

- [DsVeosCoSim_EthMessageReceivedCallback](../functions/DsVeosCoSim_EthMessageReceivedCallback.md)
- [DsVeosCoSim_ReceiveEthMessage](../functions/DsVeosCoSim_ReceiveEthMessage.md)
- [DsVeosCoSim_TransmitEthMessage](../functions/DsVeosCoSim_TransmitEthMessage.md)
