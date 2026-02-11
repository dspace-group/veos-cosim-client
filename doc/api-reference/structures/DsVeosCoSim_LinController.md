# DsVeosCoSim_LinController

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_LinController](#dsveoscosim_lincontroller)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about a LIN controller.

## Syntax

```c
typedef struct DsVeosCoSim_LinController {
    DsVeosCoSim_BusControllerId id;
    uint32_t queueSize;
    uint64_t bitsPerSecond;
    DsVeosCoSim_LinControllerType type;
    const char* name;
    const char* channelName;
    const char* clusterName;
} DsVeosCoSim_LinController;
```

## Members

> [DsVeosCoSim_BusControllerId](../simple-types/DsVeosCoSim_BusControllerId.md) id

The unique identifier of the LIN controller.

> uint32_t queueSize

The maximum queue size of the LIN controller.

> uint64_t bitsPerSecond

The baud rate of the LIN controller.

> [DsVeosCoSim_LinControllerType](../enumerations/DsVeosCoSim_LinControllerType.md) type

The LIN controller type.

> const char* name

The name of the LIN controller.

> const char* channelName

The name of the LIN channel.

> const char* clusterName

The name of the LIN cluster.

## See Also

- [DsVeosCoSim_LinMessageReceivedCallback](../functions/DsVeosCoSim_LinMessageReceivedCallback.md)
- [DsVeosCoSim_LinMessageContainerReceivedCallback](../functions/DsVeosCoSim_LinMessageContainerReceivedCallback.md)
- [DsVeosCoSim_GetLinControllers](../functions/DsVeosCoSim_GetLinControllers.md)
