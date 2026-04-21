# DsVeosCoSim_FrMessageToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_FrMessageToString](#dsveoscosim_frmessagetostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats a FlexRay message as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_FrMessageToString(
    const DsVeosCoSim_FrMessage* message
);
```

## Parameters

> const [DsVeosCoSim_FrMessage](../structures/DsVeosCoSim_FrMessage.md)* message

The FlexRay message to format.

## Return values

> std::string

A string representation of the FlexRay message.
