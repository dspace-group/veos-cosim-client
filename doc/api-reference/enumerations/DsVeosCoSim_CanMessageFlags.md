# DsVeosCoSim_CanMessageFlags

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_CanMessageFlags](#dsveoscosim_canmessageflags)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains the possible flags of a CAN message.

## Syntax

```c
typedef uint32_t DsVeosCoSim_CanMessageFlags;

enum {
    DsVeosCoSim_CanMessageFlags_Loopback = 1,
    DsVeosCoSim_CanMessageFlags_Error = 2,
    DsVeosCoSim_CanMessageFlags_Drop = 4,
    DsVeosCoSim_CanMessageFlags_ExtendedId = 8,
    DsVeosCoSim_CanMessageFlags_BitRateSwitch = 16,
    DsVeosCoSim_CanMessageFlags_FlexibleDataRateFormat = 32,
};
```

## Values

> DsVeosCoSim_CanMessageFlags_Loopback

For transmit and receive messages. Indicates that the CAN message is transmitted back to the sender as well.

> DsVeosCoSim_CanMessageFlags_Error

Only for receive messages. Indicates that the CAN message transmission failed due to an error from the VEOS CoSim server.

> DsVeosCoSim_CanMessageFlags_Drop

Only for receive messages. Indicates that the CAN message was dropped due to a full buffer at the VEOS CoSim server.

> DsVeosCoSim_CanMessageFlags_ExtendedId

For transmit and receive messages. Indicates that the CAN message uses the extended ID range.

> DsVeosCoSim_CanMessageFlags_BitRateSwitch

For transmit and receive messages. Indicates that the CAN message has a bit rate switch.

> DsVeosCoSim_CanMessageFlags_FlexibleDataRateFormat

For transmit and receive messages. Indicates a CAN FD message.

## See Also

- [DsVeosCoSim_CanMessage](../structures/DsVeosCoSim_CanMessage.md)
- [DsVeosCoSim_CanMessageContainer](../structures/DsVeosCoSim_CanMessageContainer.md)
