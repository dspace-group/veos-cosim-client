# DsVeosCoSim_WriteLinMessageContainerToMessage

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_WriteLinMessageContainerToMessage](#dsveoscosim_writelinmessagecontainertomessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Converts a LIN message container to a LIN message.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_WriteLinMessageContainerToMessage(
    const DsVeosCoSim_LinMessageContainer* messageContainer,
    DsVeosCoSim_LinMessage* message
);
```

## Parameters

> const [DsVeosCoSim_LinMessageContainer](../structures/DsVeosCoSim_LinMessageContainer.md)* messageContainer

The LIN message container to convert.

> [DsVeosCoSim_LinMessage](../structures/DsVeosCoSim_LinMessage.md)* message

The converted LIN message as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
