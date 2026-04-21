# DsVeosCoSim_WriteEthMessageContainerToMessage

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_WriteEthMessageContainerToMessage](#dsveoscosim_writeethmessagecontainertomessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Converts an Ethernet message container to an Ethernet message.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_WriteEthMessageContainerToMessage(
    const DsVeosCoSim_EthMessageContainer* messageContainer,
    DsVeosCoSim_EthMessage* message
);
```

## Parameters

> const [DsVeosCoSim_EthMessageContainer](../structures/DsVeosCoSim_EthMessageContainer.md)* messageContainer

The Ethernet message container to convert.

> [DsVeosCoSim_EthMessage](../structures/DsVeosCoSim_EthMessage.md)* message

The converted Ethernet message as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
