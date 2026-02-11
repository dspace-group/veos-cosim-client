# DsVeosCoSim_TransmitEthMessage

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_TransmitEthMessage](#dsveoscosim_transmitethmessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Transmits an Ethernet message to the VEOS CoSim server.

The `DsVeosCoSim_TransmitEthMessage` function can be called in any callback handler. The timestamp property of transmission messages will be ignored.

> [!NOTE]
> Currently, the DsVeosCoSim client cannot buffer more than 512 bus messages for each bus controller.
> If the buffer is full, the `DsVeosCoSim_TransmitEthMessage` function returns [DsVeosCoSim_Result_Full](../enumerations/DsVeosCoSim_Result.md).

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_TransmitEthMessage(
    DsVeosCoSim_Handle handle,
    const DsVeosCoSim_EthMessage* message
);
```

## Parameters

> [DsVeosCoSim_Handle](../simple-types/DsVeosCoSim_Handle.md) handle

The handle of the VEOS CoSim client.

> const [DsVeosCoSim_EthMessage](../structures/DsVeosCoSim_EthMessage.md)* message

A pointer to an Ethernet message to be transmitted.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
