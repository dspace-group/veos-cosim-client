# Messages vs. Message Containers

> [⬆️ Go to Guides](guides.md)

- [Messages vs. Message Containers](#messages-vs-message-containers)
  - [Overview](#overview)
  - [When to Use Messages](#when-to-use-messages)
  - [When to Use Message Containers](#when-to-use-message-containers)
  - [Selection Rules](#selection-rules)
  - [Recommended Approach](#recommended-approach)

## Overview

For each supported bus type, DsVeosCoSim offers two related API families:

- message APIs, for example [DsVeosCoSim_ReceiveCanMessage](../api-reference/functions/DsVeosCoSim_ReceiveCanMessage.md)
- message-container APIs, for example [DsVeosCoSim_ReceiveCanMessageContainer](../api-reference/functions/DsVeosCoSim_ReceiveCanMessageContainer.md)

Both represent received or transmitted bus data, but they use different public data structures.

## When to Use Messages

Use plain message APIs if:

- you already work with the plain message structures in your application
- you do not need the container form used by the corresponding container APIs
- you want to keep the data model aligned with the plain message callbacks

## When to Use Message Containers

Use message-container APIs if:

- you already consume the corresponding `*MessageContainer` structures
- you want your callbacks and receive APIs to use the container form consistently
- your integration benefits from a fixed container representation for that bus type

## Selection Rules

- Choose one representation per bus type per connection when possible.
- Do not register both a message callback and a message-container callback for the same bus type unless you explicitly want the container callback to take precedence.
- Do not mix receive callbacks and pull-based receive functions for the same data kind on the same connection.

## Recommended Approach

Pick one of these patterns and keep it consistent:

1. Callback-based processing with plain message callbacks
2. Callback-based processing with message-container callbacks
3. Polling or pull-based processing with plain receive functions
4. Polling or pull-based processing with container receive functions
