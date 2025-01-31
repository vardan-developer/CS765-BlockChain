#ifndef EVENT_H
#define EVENT_H

#include <time.h>
#include "block.hpp"
#include "transaction.hpp"

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
        transaction(nullptr),
        timestamp(timestamp)
    {
        this->block = new Block(block);
    }

    Event(EventType type, const Transaction & transaction, time_t timestamp):
        type(type),
        block(nullptr),
        timestamp(timestamp)
    {
        this->transaction = new Transaction(transaction);
    }

    Event(const Event & other) {
        type = other.type;
        timestamp = other.timestamp;
        if (other.block != nullptr) {
            block = new Block(*other.block);
            transaction = nullptr;
        } else {
            transaction = new Transaction(*other.transaction);
            block = nullptr;
        }
    };

    Event & operator = (const Event & other) {
        if (this == &other) {
            return *this;
        }
        type = other.type;
        timestamp = other.timestamp;
        if (other.block != nullptr) {
            block = new Block(*other.block);
            transaction = nullptr;
        } else {
            transaction = new Transaction(*other.transaction);
            block = nullptr;
        }
        return *this;
    }

    bool operator < (const Event& other) const {
        return timestamp < other.timestamp;
    }

    ~Event() {
        delete block;
        delete transaction;
    }
    
};

#endif