# Callback Ordering and Buffering

> [⬆️ Go to Guides](guides.md)

- [Callback Ordering and Buffering](#callback-ordering-and-buffering)
  - [Callback Order Within a Step](#callback-order-within-a-step)
  - [Callback Precedence](#callback-precedence)
  - [Receive API Availability](#receive-api-availability)
  - [Polling-Based Behavior](#polling-based-behavior)

## Callback Order Within a Step

Within a simulation step, callbacks are observed in this logical order:

1. Begin-step callback
2. Data callbacks for incoming signals and received bus data
3. End-step callback

The exact order among data callbacks within the same step is not specified.

## Callback Precedence

For each supported bus type, message-container callbacks take precedence over message callbacks.

If both callbacks are registered for the same bus type:

- the message-container callback is called
- the plain message callback is not called for that data

This applies to:

- CAN
- Ethernet
- LIN
- FlexRay

## Receive API Availability

When a receive callback is registered for a bus type, that bus data is no longer intended to be collected through the corresponding receive functions for the same connection.

Practical rule:

- Use callbacks for push-style processing.
- Use receive functions for pull-style processing.
- Do not mix both styles for the same data kind on the same connection.

## Polling-Based Behavior

Polling-based co-simulation still uses the same callback registration rules.

- State-change callbacks and data callbacks may still be delivered.
- `PollCommand` controls when your application processes the next server command.
- Each successful `PollCommand` must be followed by `FinishCommand`.
