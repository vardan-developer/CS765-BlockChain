# Design

## Design

- Each node has its own private tree

## 1. Events

- Block Creation
- Braodcast

## Block Creation

- if a node crates a block at T_0 time, we will add it to the evvent list of the node and return this back when the event loop in main thread is called.
- then the main thread will add it to the main queue of the events.
- When the main thread encounters a block creation event, it will ping the owner to ask if he still wants to add the block as it may be outdated.
- If owner says yes, then main thread will schedule the block bradcast event to be executed at T_0 time.


## Broadcast

- When the main thread encounters a broadcast event, the main thread we will schedule send message event for each of the neighbour nodes with added latency for each node.

## Send Message

- When the main thread encounters a send message event, it will send the message to the neighbour node immendiately with the timestamp written in the message.
