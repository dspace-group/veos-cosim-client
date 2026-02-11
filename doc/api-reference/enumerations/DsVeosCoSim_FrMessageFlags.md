# DsVeosCoSim_FrMessageFlags

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_FrMessageFlags](#dsveoscosim_frmessageflags)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains the possible flags of a FlexRay message.

## Syntax

```c
typedef uint32_t DsVeosCoSim_FrMessageFlags;

enum {
    DsVeosCoSim_FrMessageFlags_Loopback = 1,
    DsVeosCoSim_FrMessageFlags_Error = 2,
    DsVeosCoSim_FrMessageFlags_Drop = 4,
    DsVeosCoSim_FrMessageFlags_Startup = 8,
    DsVeosCoSim_FrMessageFlags_SyncFrame = 16,
    DsVeosCoSim_FrMessageFlags_NullFrame = 32,
    DsVeosCoSim_FrMessageFlags_PayloadPreamble = 64,
    DsVeosCoSim_FrMessageFlags_TransferOnce = 128,
    DsVeosCoSim_FrMessageFlags_ChannelA = 256,
    DsVeosCoSim_FrMessageFlags_ChannelB = 512
};
```

## Values

> DsVeosCoSim_FrMessageFlags_Loopback

For transmit and receive messages. Indicates that the FlexRay message is transmitted back to the sender as well.

> DsVeosCoSim_FrMessageFlags_Error

Only for receive messages. Indicates that the FlexRay message transmission failed due to an error from the VEOS CoSim server.

> DsVeosCoSim_FrMessageFlags_Drop

Only for receive messages. Indicates that the FlexRay message was dropped due to a full buffer at the VEOS CoSim server.

> DsVeosCoSim_FrMessageFlags_Startup

For transmit and receive messages. FlexRay message is a startup frame.

> DsVeosCoSim_FrMessageFlags_SyncFrame

For transmit and receive messages. FlexRay message is a sync frame.

> DsVeosCoSim_FrMessageFlags_NullFrame

For transmit and receive messages. FlexRay message is a null frame.

> DsVeosCoSim_FrMessageFlags_PayloadPreamble

For transmit and receive messages. FlexRay message uses the payload preamble.

> DsVeosCoSim_FrMessageFlags_TransferOnce

Only for transmit messages.
FlexRay message will only be transmitted in one cycle, otherwise message will be retransmitted in every upcoming cycle.

> DsVeosCoSim_FrMessageFlags_ChannelA

For transmit and receive messages. FlexRay message is transmitted on Channel A.

> DsVeosCoSim_FrMessageFlags_ChannelB

For transmit and receive messages. FlexRay message is transmitted on Channel B.

## See Also

- [DsVeosCoSim_FrMessage](../structures/DsVeosCoSim_FrMessage.md)
- [DsVeosCoSim_FrMessageContainer](../structures/DsVeosCoSim_FrMessageContainer.md)
