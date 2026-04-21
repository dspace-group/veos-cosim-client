# DsVeosCoSim_WriteLinMessageToMessageContainer

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_WriteLinMessageToMessageContainer](#dsveoscosim_writelinmessagetomessagecontainer)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Converts a LIN message to a LIN message container.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_WriteLinMessageToMessageContainer(
    const DsVeosCoSim_LinMessage* message,
    DsVeosCoSim_LinMessageContainer* messageContainer
);
```

## Parameters

> const [DsVeosCoSim_LinMessage](../structures/DsVeosCoSim_LinMessage.md)* message

The LIN message to convert.

> [DsVeosCoSim_LinMessageContainer](../structures/DsVeosCoSim_LinMessageContainer.md)* messageContainer

The converted LIN message container as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
