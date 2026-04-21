# DsVeosCoSim_CanMessageToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_CanMessageToString](#dsveoscosim_canmessagetostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a CAN message as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_CanMessageToString(
    const DsVeosCoSim_CanMessage* message
);
```

## Parameters

> const [DsVeosCoSim_CanMessage](../structures/DsVeosCoSim_CanMessage.md)* message

The CAN message to format.

## Return values

> std::string

A string representation of the CAN message.
