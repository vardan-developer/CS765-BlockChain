#ifndef EVENT_H
#define EVENT_H

#include <ctime>
#include <ostream>
#include <time.h>
#include "block.hpp"
#include "def.hpp"
#include "transaction.hpp"

/**
 * @brief Enum class representing different types of events in the system
 * 
 * RECEIVE_BROADCAST_TRANSACTION: Event when a broadcast transaction is received
 * RECEIVE_BROADCAST_BLOCK: Event when a broadcast block is received
 * BLOCK_CREATION: Event for creating a new block
 * BROADCAST_BLOCK: Event for broadcasting a block
 * BROADCAST_TRANSACTION: Event for broadcasting a transaction
 * SEND_BLOCK: Event for sending a broadcast block
 * SEND_TRANSACTION: Event for sending a broadcast transaction
 */



enum class EventType {
    SEND_TRANSACTION,
    RECEIVE_TRANSACTION,
    SEND_GET,
    RECEIVE_GET,
    SEND_BLOCK,
    RECEIVE_BLOCK,
    SEND_HASH,
    RECEIVE_HASH,
    BROADCAST_PRIVATE_CHAIN,
    BLOCK_CREATION
};

struct Event {
    EventType type;
    time_t timestamp;
    minerID_t owner, sender, receiver;
    bool broadcast, malicious;
    
    Event(EventType type, time_t timestamp, minerID_t owner, minerID_t sender, minerID_t receiver, bool broadcast, bool malicious):
        type(type), timestamp(timestamp), owner(owner), sender(sender), receiver(receiver), broadcast(broadcast), malicious(malicious) {}

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
    friend std::ostream& operator<< (std::ostream& stream, const Event& event) {
        stream << "Event: " << static_cast<std::underlying_type<EventType>::type>(event.type) << ", Timestamp: " << event.timestamp << ", Owner: " << event.owner << ", Sender: " << event.sender << ", Receiver: " << event.receiver << ", Broadcast: " << event.broadcast << ", Malicious: " << event.malicious << std::endl;
        return stream;
    }
};

struct BlockEvent: Event {
    Block block;
    BlockEvent(EventType type, const Block block, time_t timestamp, minerID_t owner, minerID_t sender, minerID_t receiver, bool broadcast = false, bool malicious = false):
        Event(type, timestamp, owner, sender, receiver, broadcast, malicious), block(block) {}
};

struct TransactionEvent: Event {
    Transaction transaction;
    TransactionEvent(EventType type, const Transaction transaction, time_t timestamp, minerID_t owner, minerID_t sender, minerID_t receiver, bool broadcast = false, bool malicious = false):
        Event(type, timestamp, owner, sender, receiver, broadcast, malicious), transaction(transaction) {}
};

struct HashEvent: Event {
    hash_t hash;
    HashEvent(EventType type, const hash_t hash, time_t timestamp, minerID_t owner, minerID_t sender, minerID_t receiver, bool broadcast = false, bool malicious = false):
        Event(type, timestamp, owner, sender, receiver, broadcast, malicious), hash(hash) {}
};

using GetEvent = HashEvent;

struct BroadcastPrivateChainEvent: Event {
    blockID_t block_id;
    BroadcastPrivateChainEvent(EventType type, blockID_t block_id, time_t timestamp, minerID_t owner, minerID_t sender, minerID_t receiver, bool broadcast = false, bool malicious = false):
        Event(type, timestamp, owner, sender, receiver, broadcast, malicious), block_id(block_id) {}
};

/**
 * @brief Class representing events in the blockchain system
 * Contains either a Block or Transaction, but never both simultaneously
 */
// struct Event {
//     EventType type;          // Type of event (from EventType enum)
//     Block * block;          // Pointer to block (if block-related event)
//     Transaction * transaction; // Pointer to transaction (if transaction-related event)
//     time_t timestamp;       // Scheduled processing time for this event
//     minerID_t owner;        // ID of the miner who created this event
//     minerID_t receiver;     // ID of the intended receiving miner
//     bool broadcast;
//     bool malicious;
//     hash_t hash;

//     // Constructor for block-related events
//     // Creates a deep copy of the provided block
//     Event(EventType type, const Block* block, time_t timestamp, minerID_t owner, minerID_t receiver, bool broadcast = false, bool malicious = false)
//         : type(type),
//           block(new Block(*block)),
//           transaction(nullptr),
//           timestamp(timestamp),
//           owner(owner),
//           receiver(receiver),
//           broadcast(broadcast),
//           malicious(malicious) {}

//     // Constructor for transaction-related events
//     // Creates a deep copy of the provided transaction
//     Event(EventType type, const Transaction* transaction, time_t timestamp, minerID_t owner, minerID_t receiver, bool broadcast = false, bool malicious = false)
//         : type(type),
//           block(nullptr),
//           transaction(new Transaction(*transaction)),
//           timestamp(timestamp),
//           owner(owner),
//           receiver(receiver),
//           broadcast(broadcast),
//           malicious(malicious) {}

//     Event(EventType type): type(type) {}

//     // Copy constructor
//     // Performs deep copy of either block or transaction
//     Event(const Event & other) {
//         type = other.type;
//         timestamp = other.timestamp;
//         owner = other.owner;
//         receiver = other.receiver;
//         if (other.block != nullptr) {
//             block = new Block(*other.block);
//             transaction = nullptr;
//         } else {
//             transaction = new Transaction(*other.transaction);
//             block = nullptr;
//         }
//     };

//     // Copy assignment operator
//     // Performs deep copy of either block or transaction
//     Event & operator = (const Event & other) {
//         if (this == &other) {
//             return *this;
//         }
//         type = other.type;
//         timestamp = other.timestamp;
//         owner = other.owner;
//         receiver = other.receiver;
//         if (other.block != nullptr) {
//             block = new Block(*other.block);
//             transaction = nullptr;
//         } else {
//             transaction = new Transaction(*other.transaction);
//             block = nullptr;
//         }
//         return *this;
//     }

//     // Move constructor
//     // Transfers ownership of block or transaction
//     Event(Event && other) {
//         type = other.type;
//         timestamp = other.timestamp;
//         owner = other.owner;
//         receiver = other.receiver;
//         if (other.block != nullptr) {
//             this->block = other.block;
//             other.block = nullptr;
//             this->transaction = nullptr;
//         } else {
//             this->transaction = other.transaction;
//             other.transaction = nullptr;
//             this->block = nullptr;
//         }
//     }

//     // Move assignment operator
//     // Transfers ownership of block or transaction
//     Event & operator = (Event && other) {
//         if (this==&other) {
//             return *this;
//         }
//         type = other.type;
//         timestamp = other.timestamp;
//         owner = other.owner;
//         receiver = other.receiver;
//         if (other.block != nullptr) {
//             this->block = other.block;
//             other.block = nullptr;
//             this->transaction = nullptr;
//         } else {
//             this->transaction = other.transaction;
//             other.transaction = nullptr;
//             this->block = nullptr;
//         }
//         return *this;
//     }

//     // Comparison operators for event ordering based on timestamp
//     // Used for priority queue implementation
//     bool operator < (const Event& other) const {
//         return timestamp < other.timestamp;
//     }

//     bool operator == (const Event& other) const {
//         return timestamp == other.timestamp;
//     }

//     bool operator != (const Event& other) const {
//         return timestamp != other.timestamp;
//     }

//     bool operator > (const Event& other) const {
//         return timestamp > other.timestamp;
//     }

//     // Destructor
//     // Properly cleans up dynamically allocated block or transaction
//     ~Event() {
//         if ( block ) {
//             delete block;
//         }
//         if ( transaction ) {
//             delete transaction;
//         }
//     }
// };

#endif