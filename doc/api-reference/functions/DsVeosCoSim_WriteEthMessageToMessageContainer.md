# DsVeosCoSim_WriteEthMessageToMessageContainer

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_WriteEthMessageToMessageContainer](#dsveoscosim_writeethmessagetomessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Converts an Ethernet message to an Ethernet message container.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_WriteEthMessageToMessageContainer(
    const DsVeosCoSim_EthMessage* message,
    DsVeosCoSim_EthMessageContainer* messageContainer
);
```

## Parameters

> const [DsVeosCoSim_EthMessage](../structures/DsVeosCoSim_EthMessage.md)* message

The Ethernet message to convert.

> [DsVeosCoSim_EthMessageContainer](../structures/DsVeosCoSim_EthMessageContainer.md)* messageContainer

The converted Ethernet message container as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
