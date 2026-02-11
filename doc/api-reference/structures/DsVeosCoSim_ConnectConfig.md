# DsVeosCoSim_ConnectConfig

> [⬆️ Go to Structures](structures.md)

- [DsVeosCoSim\_ConnectConfig](#dsveoscosim_connectconfig)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Members](#members)
  - [See Also](#see-also)

## Description

Contains the data that is required for establishing a connection to VEOS.

## Syntax

```c
typedef struct DsVeosCoSim_ConnectConfig {
    const char* remoteIpAddress;
    const char* serverName;
    const char* clientName;
    uint16_t remotePort;
    uint16_t localPort;
} DsVeosCoSim_ConnectConfig;
```

## Members

> const char* remoteIpAddress

The IP address or host name of the VEOS CoSim server.

> const char* serverName

The name of the VEOS CoSim server.

> const char* clientName

The name of the VEOS CoSim Client.

> uint16_t remotePort

The TCP port of the VEOS CoSim server.

> uint16_t localPort

The port of the VEOS CoSim client. Only change this value if tunneled communication is required.

## See Also

- [DsVeosCoSim_Connect](../functions/DsVeosCoSim_Connect.md)
