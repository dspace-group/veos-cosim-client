# DsVeosCoSim_EthController

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_EthController](#dsveoscosim_ethcontroller)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains information about an Ethernet controller.

## Syntax

```c
typedef struct DsVeosCoSim_EthController {
    DsVeosCoSim_BusControllerId id;
    uint32_t queueSize;
    uint64_t bitsPerSecond;
    uint8_t macAddress[DSVEOSCOSIM_ETH_ADDRESS_LENGTH];
    const char* name;
    const char* channelName;
    const char* clusterName;
} DsVeosCoSim_EthController;
```

## Members

> [DsVeosCoSim_BusControllerId](../simple-types/DsVeosCoSim_BusControllerId.md) id

The unique identifier of the Ethernet controller.

> uint32_t queueSize

The maximum queue size of the Ethernet controller.

> uint64_t bitsPerSecond

The baud rate of the Ethernet controller.

> uint8_t macAddress[[DSVEOSCOSIM_ETH_ADDRESS_LENGTH](../macros/DSVEOSCOSIM_ETH_ADDRESS_LENGTH.md)]

The MAC address of the Ethernet controller.

> const char* name

The name of the Ethernet controller.

> const char* channelName

The name of the Ethernet channel.

> const char* clusterName

The name of the Ethernet cluster.

## See Also

- [DsVeosCoSim_EthMessageReceivedCallback](../functions/DsVeosCoSim_EthMessageReceivedCallback.md)
- [DsVeosCoSim_EthMessageContainerReceivedCallback](../functions/DsVeosCoSim_EthMessageContainerReceivedCallback.md)
- [DsVeosCoSim_GetEthControllers](../functions/DsVeosCoSim_GetEthControllers.md)
