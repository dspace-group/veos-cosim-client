# DsVeosCoSim_WriteCanMessageToMessageContainer

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_WriteCanMessageToMessageContainer](#dsveoscosim_writecanmessagetomessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Converts a CAN message to a CAN message container.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_WriteCanMessageToMessageContainer(
    const DsVeosCoSim_CanMessage* message,
    DsVeosCoSim_CanMessageContainer* messageContainer
);
```

## Parameters

> const [DsVeosCoSim_CanMessage](../structures/DsVeosCoSim_CanMessage.md)* message

The CAN message to convert.

> [DsVeosCoSim_CanMessageContainer](../structures/DsVeosCoSim_CanMessageContainer.md)* messageContainer

The converted CAN message container as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
