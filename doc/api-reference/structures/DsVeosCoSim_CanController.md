# DsVeosCoSim_CanController

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_CanController](#dsveoscosim_cancontroller)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about a CAN controller.

## Syntax

```c
typedef struct DsVeosCoSim_CanController {
    DsVeosCoSim_BusControllerId id;
    uint32_t queueSize;
    uint64_t bitsPerSecond;
    uint64_t flexibleDataRateBitsPerSecond;
    const char* name;
    const char* channelName;
    const char* clusterName;
} DsVeosCoSim_CanController;
```

## Members

> [DsVeosCoSim_BusControllerId](../simple-types/DsVeosCoSim_BusControllerId.md) id

The unique identifier of the CAN controller.

> uint32_t queueSize

The maximum queue size of the CAN controller.

> uint64_t bitsPerSecond

The baud rate of the CAN controller.

> uint64_t flexibleDataRateBitsPerSecond

The baud rate of the CAN controller for CAN FD.

> const char* name

The name of the CAN controller.

> const char* channelName

The name of the CAN channel.

> const char* clusterName

The name of the CAN cluster.

## See Also

- [DsVeosCoSim_CanMessageReceivedCallback](../functions/DsVeosCoSim_CanMessageReceivedCallback.md)
- [DsVeosCoSim_CanMessageContainerReceivedCallback](../functions/DsVeosCoSim_CanMessageContainerReceivedCallback.md)
- [DsVeosCoSim_GetCanControllers](../functions/DsVeosCoSim_GetCanControllers.md)
