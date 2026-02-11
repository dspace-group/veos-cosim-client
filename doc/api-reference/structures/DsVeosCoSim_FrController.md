# DsVeosCoSim_FrController

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_FrController](#dsveoscosim_frcontroller)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about a FlexRay controller.

## Syntax

```c
typedef struct DsVeosCoSim_FrController {
    DsVeosCoSim_BusControllerId id;
    uint32_t queueSize;
    uint64_t bitsPerSecond;
    const char* name;
    const char* channelName;
    const char* clusterName;
} DsVeosCoSim_FrController;
```

## Members

> [DsVeosCoSim_BusControllerId](../simple-types/DsVeosCoSim_BusControllerId.md) id

The unique identifier of the FlexRay controller.

> uint32_t queueSize

The maximum queue size of the FlexRay controller.

> uint64_t bitsPerSecond

The baud rate of the FlexRay controller.

> const char* name

The name of the FlexRay controller.

> const char* channelName

The name of the FlexRay channel.

> const char* clusterName

The name of the FlexRay cluster.

## See Also

- [DsVeosCoSim_FrMessageReceivedCallback](../functions/DsVeosCoSim_FrMessageReceivedCallback.md)
- [DsVeosCoSim_FrMessageContainerReceivedCallback](../functions/DsVeosCoSim_FrMessageContainerReceivedCallback.md)
- [DsVeosCoSim_GetFrControllers](../functions/DsVeosCoSim_GetFrControllers.md)
