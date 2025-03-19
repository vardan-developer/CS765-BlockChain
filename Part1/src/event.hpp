#ifndef EVENT_H
#define EVENT_H

#include <time.h>
#include "block.hpp"
#include "transaction.hpp"

/**
 * @brief Enum class representing different types of events in the system
 * 
 * RECEIVE_BROADCAST_TRANSACTION: Event when a broadcast transaction is received
 * RECEIVE_BROADCAST_BLOCK: Event when a broadcast block is received
 * BLOCK_CREATION: Event for creating a new block
 * BROADCAST_BLOCK: Event for broadcasting a block
 * BROADCAST_TRANSACTION: Event for broadcasting a transaction
 * SEND_BROADCAST_BLOCK: Event for sending a broadcast block
 * SEND_BROADCAST_TRANSACTION: Event for sending a broadcast transaction
 */
enum class EventType {
    RECEIVE_BROADCAST_TRANSACTION,
    RECEIVE_BROADCAST_BLOCK,
    BLOCK_CREATION,
    BROADCAST_BLOCK,
    BROADCAST_TRANSACTION,
    SEND_BROADCAST_BLOCK,
    SEND_BROADCAST_TRANSACTION
};

/**
 * @brief Class representing events in the blockchain system
 * Contains either a Block or Transaction, but never both simultaneously
 */
struct Event {
    EventType type;          // Type of event (from EventType enum)
    Block * block;          // Pointer to block (if block-related event)
    Transaction * transaction; // Pointer to transaction (if transaction-related event)
    time_t timestamp;       // Scheduled processing time for this event
    minerID_t owner;        // ID of the miner who created this event
    minerID_t receiver;     // ID of the intended receiving miner

    // Constructor for block-related events
    // Creates a deep copy of the provided block
    Event(EventType type, const Block* block, time_t timestamp, minerID_t owner, minerID_t receiver)
        : type(type),
          block(nullptr),
          transaction(nullptr),
          timestamp(timestamp),
          owner(owner),
          receiver(receiver)
    {
        this->block = new Block(*block);
    }

    // Constructor for transaction-related events
    // Creates a deep copy of the provided transaction
    Event(EventType type, const Transaction* transaction, time_t timestamp, minerID_t owner, minerID_t receiver)
        : type(type),
          block(nullptr),
          timestamp(timestamp),
          owner(owner),
          receiver(receiver)
    {
        this->transaction = new Transaction(*transaction);
    }

    // Copy constructor
    // Performs deep copy of either block or transaction
    Event(const Event & other) {
        type = other.type;
        timestamp = other.timestamp;
        owner = other.owner;
        receiver = other.receiver;
        if (other.block != nullptr) {
            block = new Block(*other.block);
            transaction = nullptr;
        } else {
            transaction = new Transaction(*other.transaction);
            block = nullptr;
        }
    };

    // Copy assignment operator
    // Performs deep copy of either block or transaction
    Event & operator = (const Event & other) {
        if (this == &other) {
            return *this;
        }
        type = other.type;
        timestamp = other.timestamp;
        owner = other.owner;
        receiver = other.receiver;
        if (other.block != nullptr) {
            block = new Block(*other.block);
            transaction = nullptr;
        } else {
            transaction = new Transaction(*other.transaction);
            block = nullptr;
        }
        return *this;
    }

    // Move constructor
    // Transfers ownership of block or transaction
    Event(Event && other) {
        type = other.type;
        timestamp = other.timestamp;
        owner = other.owner;
        receiver = other.receiver;
        if (other.block != nullptr) {
            this->block = other.block;
            other.block = nullptr;
            this->transaction = nullptr;
        } else {
            this->transaction = other.transaction;
            other.transaction = nullptr;
            this->block = nullptr;
        }
    }

    // Move assignment operator
    // Transfers ownership of block or transaction
    Event & operator = (Event && other) {
        if (this==&other) {
            return *this;
        }
        type = other.type;
        timestamp = other.timestamp;
        owner = other.owner;
        receiver = other.receiver;
        if (other.block != nullptr) {
            this->block = other.block;
            other.block = nullptr;
            this->transaction = nullptr;
        } else {
            this->transaction = other.transaction;
            other.transaction = nullptr;
            this->block = nullptr;
        }
        return *this;
    }

    // Comparison operators for event ordering based on timestamp
    // Used for priority queue implementation
    bool operator < (const Event& other) const {
        return timestamp < other.timestamp;
    }

    bool operator == (const Event& other) const {
        return timestamp == other.timestamp;
    }

    bool operator != (const Event& other) const {
        return timestamp != other.timestamp;
    }

    bool operator > (const Event& other) const {
        return timestamp > other.timestamp;
    }

    // Destructor
    // Properly cleans up dynamically allocated block or transaction
    ~Event() {
        if ( block ) {
            delete block;
        }
        if ( transaction ) {
            delete transaction;
        }
    }
};

#endif