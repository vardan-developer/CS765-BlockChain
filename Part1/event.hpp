#ifndef EVENT_H
#define EVENT_H

#include <time.h>
#include "block.h"
#include "transaction.h"

enum class EventType {
    RECEIVE_TRANSACTION,
    RECEIVE_BLOCK,
    BLOCK_CREATION,
    BROADCAST_BLOCK,
    BROADCAST_TRANSACTION
};

struct Event {
    EventType type;
    Block * block;
    Transaction * transaction;
    time_t timestamp;       // Time when this event will be processed

    Event(EventType type, const Block & block, time_t timestamp):
        type(type),
        timestamp(timestamp)
    {
        this->block = new Block(block);
    }

    Event(EventType type, const Transaction & transaction, time_t timestamp):
        type(type),
        timestamp(timestamp)
    {
        this->transaction = new Transaction(transaction);
    }

    bool operator < (const Event& other) const {
        return timestamp < other.timestamp;
    }
    
};

#endif