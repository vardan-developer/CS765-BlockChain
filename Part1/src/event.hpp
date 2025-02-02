#ifndef EVENT_H
#define EVENT_H

#include <time.h>
#include "block.hpp"
#include "transaction.hpp"

enum class EventType {
    RECEIVE_BROADCAST_TRANSACTION,
    RECEIVE_BROADCAST_BLOCK,
    BLOCK_CREATION,
    BROADCAST_BLOCK,
    BROADCAST_TRANSACTION,
    SEND_BROADCAST_BLOCK,
    SEND_BROADCAST_TRANSACTION
};

struct Event {
    EventType type;
    Block * block;
    Transaction * transaction;
    time_t timestamp;       // Time when this event will be processed
    minerId_t owner;

    Event(EventType type, const Block*  block, time_t timestamp, minerId_t owner):
        type(type),
        transaction(nullptr),
        timestamp(timestamp),
        owner(owner)
    {
        this->block = new Block(*block);
    }

    Event(EventType type, const Transaction*  transaction, time_t timestamp, minerId_t owner):
        type(type),
        block(nullptr),
        timestamp(timestamp),
        owner(owner)
    {
        this->transaction = new Transaction(*transaction);
    }

    Event(const Event & other) {
        type = other.type;
        timestamp = other.timestamp;
        owner = other.owner;
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
        owner = other.owner;
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