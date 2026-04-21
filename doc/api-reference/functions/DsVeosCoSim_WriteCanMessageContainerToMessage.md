# DsVeosCoSim_WriteCanMessageContainerToMessage

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_WriteCanMessageContainerToMessage](#dsveoscosim_writecanmessagecontainertomessage)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Converts a CAN message container to a CAN message.

## Syntax

```c
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_WriteCanMessageContainerToMessage(
    const DsVeosCoSim_CanMessageContainer* messageContainer,
    DsVeosCoSim_CanMessage* message
);
```

## Parameters

> const [DsVeosCoSim_CanMessageContainer](../structures/DsVeosCoSim_CanMessageContainer.md)* messageContainer

The CAN message container to convert.

> [DsVeosCoSim_CanMessage](../structures/DsVeosCoSim_CanMessage.md)* message

The converted CAN message as an out parameter.

## Return values

A [DsVeosCoSim_Result](../enumerations/DsVeosCoSim_Result.md).
