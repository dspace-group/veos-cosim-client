# DsVeosCoSim_LinMessageFlags

[⬆️ Go to Enumerations](enumerations.md)

- [DsVeosCoSim\_LinMessageFlags](#dsveoscosim_linmessageflags)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Values](#values)
  - [See Also](#see-also)

## Description

Contains the possible flags of a LIN message.

> [!NOTE]
> - One of the following flags must be set:
>    - `DsVeosCoSim_LinMessageFlags_Header`
>    - `DsVeosCoSim_LinMessageFlags_Response`
>    - `DsVeosCoSim_LinMessageFlags_WakeEvent`
>    - `DsVeosCoSim_LinMessageFlags_SleepEvent`
>
> - If `DsVeosCoSim_LinMessageFlags_Header` and `DsVeosCoSim_LinMessageFlags_Response` are set simultaneously, the message contains both header and response.
>
> - `DsVeosCoSim_LinMessageFlags_WakeEvent` and `DsVeosCoSim_LinMessageFlags_SleepEvent` cannot be combined with each other or with `DsVeosCoSim_LinMessageFlags_Header` and `DsVeosCoSim_LinMessageFlags_Response`

## Syntax

```c
typedef uint32_t DsVeosCoSim_LinMessageFlags;

enum {
    DsVeosCoSim_LinMessageFlags_Loopback = 1,
    DsVeosCoSim_LinMessageFlags_Error = 2,
    DsVeosCoSim_LinMessageFlags_Drop = 4,
    DsVeosCoSim_LinMessageFlags_Header = 8,
    DsVeosCoSim_LinMessageFlags_Response = 16,
    DsVeosCoSim_LinMessageFlags_WakeEvent = 32,
    DsVeosCoSim_LinMessageFlags_SleepEvent = 64,
    DsVeosCoSim_LinMessageFlags_EnhancedChecksum = 128,
    DsVeosCoSim_LinMessageFlags_TransferOnce = 256,
    DsVeosCoSim_LinMessageFlags_ParityFailure = 512,
    DsVeosCoSim_LinMessageFlags_Collision = 1024,
    DsVeosCoSim_LinMessageFlags_NoResponse = 2048,
};
```

## Values

> DsVeosCoSim_LinMessageFlags_Loopback

For transmit and receive messages. Indicates that the LIN message is transmitted back to the sender as well.

> DsVeosCoSim_LinMessageFlags_Error

Only for receive messages. Indicates that the LIN message transmission failed due to an error from the VEOS CoSim server.

> DsVeosCoSim_LinMessageFlags_Drop

Only for receive messages. Indicates that the LIN message was dropped due to a full buffer at the VEOS CoSim server.

> DsVeosCoSim_LinMessageFlags_Header

For transmit and receive messages. Indicates that the LIN message contains a header.

> DsVeosCoSim_LinMessageFlags_Response

For transmit and receive messages. Indicates that the LIN message contains a response.

> DsVeosCoSim_LinMessageFlags_WakeEvent

For transmit and receive messages. Indicates that the LIN message contains a wake command.

> DsVeosCoSim_LinMessageFlags_SleepEvent

For transmit and receive messages. Indicates that the LIN message contains a sleep command.

> DsVeosCoSim_LinMessageFlags_EnhancedChecksum

For transmit and receive messages. Indicates that the LIN message uses the enhanced checksum.

> DsVeosCoSim_LinMessageFlags_TransferOnce

Only for transmit messages. This flag only makes sense if the DsVeosCoSim_LinMessageFlags_Response flag is set. Indicates a single ad-hoc transmission of a LIN response message. If the message is not in the allowed response space it will be omitted silently.

> DsVeosCoSim_LinMessageFlags_ParityFailure

Only for receive messages. Indicates that the LIN header could not be transmitted, because another LIN header was sent at the same time.

> DsVeosCoSim_LinMessageFlags_Collision

Only for receive messages. Indicates that the LIN response could not be transmitted, because another LIN response was sent at the same time.

> DsVeosCoSim_LinMessageFlags_NoResponse

Only for receive messages. Indicates that no response to the last header was received.

## See Also

- [DsVeosCoSim_LinMessage](../structures/DsVeosCoSim_LinMessage.md)
- [DsVeosCoSim_LinMessageContainer](../structures/DsVeosCoSim_LinMessageContainer.md)
