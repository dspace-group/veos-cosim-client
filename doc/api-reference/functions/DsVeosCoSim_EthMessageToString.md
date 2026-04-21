# DsVeosCoSim_EthMessageToString

[⬆️ Go to Functions](functions.md)

- [DsVeosCoSim\_EthMessageToString](#dsveoscosim_ethmessagetostring)
  - [Description](#description)
  - [Syntax](#syntax)
  - [Parameters](#parameters)
  - [Return values](#return-values)

## Description

Formats an Ethernet message as a string.

This function is only available in C++.

## Syntax

```cpp
extern DSVEOSCOSIM_API std::string DsVeosCoSim_EthMessageToString(
    const DsVeosCoSim_EthMessage* message
);
```

## Parameters

> const [DsVeosCoSim_EthMessage](../structures/DsVeosCoSim_EthMessage.md)* message

The Ethernet message to format.

## Return values

> std::string

A string representation of the Ethernet message.
