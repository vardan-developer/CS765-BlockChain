#ifndef EVENT_H
#define EVENT_H

#include <time.h>
#include "block.h"
#include "transaction.h"

enum class EventType {
    RECEIVE_MESSAGE,
    BLOCK_CREATION,
    BROADCAST_BLOCK,
    BROADCAST_TRANSACTION
};

struct Event {
    EventType type;
    Block block;
    Transaction transaction;
    time_t timestamp;       // Time when this event will be processed

    Event(EventType type, Block block, Transaction transaction, time_t timestamp):
        type(type),
        block(block),
        transaction(transaction),
        timestamp(timestamp)
    {}

    bool operator < (const Event& other) const {
        return timestamp < other.timestamp;
    }
    
};

#endif