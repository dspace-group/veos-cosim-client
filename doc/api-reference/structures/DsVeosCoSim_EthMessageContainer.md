# DsVeosCoSim_EthMessageContainer

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_EthMessageContainer](#dsveoscosim_ethmessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about an Ethernet message container.

## Syntax

```c
typedef struct DsVeosCoSim_EthMessageContainer {
    DsVeosCoSim_SimulationTime timestamp;
    DsVeosCoSim_BusControllerId controllerId;
    uint32_t reserved;
    DsVeosCoSim_EthMessageFlags flags;
    uint32_t length;
    uint8_t data[DSVEOSCOSIM_ETH_MESSAGE_MAX_LENGTH];
} DsVeosCoSim_EthMessageContainer;
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

> uint8_t data[[DSVEOSCOSIM_ETH_MESSAGE_MAX_LENGTH](../macros/DSVEOSCOSIM_ETH_MESSAGE_MAX_LENGTH.md)]

The Ethernet message payload.

## See Also

- [DsVeosCoSim_EthMessageContainerReceivedCallback](../functions/DsVeosCoSim_EthMessageContainerReceivedCallback.md)
- [DsVeosCoSim_ReceiveEthMessageContainer](../functions/DsVeosCoSim_ReceiveEthMessageContainer.md)
- [DsVeosCoSim_TransmitEthMessageContainer](../functions/DsVeosCoSim_TransmitEthMessageContainer.md)
