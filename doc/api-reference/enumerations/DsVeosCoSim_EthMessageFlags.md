# DsVeosCoSim_EthMessageFlags

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_EthMessageFlags](#dsveoscosim_ethmessageflags)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains the possible flags of an Ethernet message.

## Syntax

```c
typedef uint32_t DsVeosCoSim_EthMessageFlags;

enum {
    DsVeosCoSim_EthMessageFlags_Loopback = 1,
    DsVeosCoSim_EthMessageFlags_Error = 2,
    DsVeosCoSim_EthMessageFlags_Drop = 4,
};
```

## Values

> DsVeosCoSim_EthMessageFlags_Loopback

For transmit and receive messages. Indicates that the Ethernet message is transmitted back to the sender as well.

> DsVeosCoSim_EthMessageFlags_Error

Only for receive messages. Indicates that the Ethernet message transmission failed due to an error from the VEOS CoSim server.

> DsVeosCoSim_EthMessageFlags_Drop

Only for receive messages. Indicates that the Ethernet message was dropped due to a full buffer at the VEOS CoSim server.

## See Also

- [DsVeosCoSim_EthMessage](../structures/DsVeosCoSim_EthMessage.md)
- [DsVeosCoSim_EthMessageContainer](../structures/DsVeosCoSim_EthMessageContainer.md)
