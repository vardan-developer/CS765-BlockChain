#include "block.hpp"

// Initialize block with default values: id=0, height=0, no parent, empty transactions, 
// timestamp=0, and invalid owner (-1)
Block::Block(): id(-1), height(0), parentID(0), transactions(), timestamp(0), owner(-1) {}

// Full constructor initializing all block members using initialization list
Block::Block(blockID_t id, uint64_t height, blockID_t parentID, 
            std::vector<Transaction> transactions, time_t timestamp, minerID_t owner)
    : id(id), height(height), parentID(parentID), 
      transactions(transactions), timestamp(timestamp), owner(owner) {}

// Constructor without transactions - creates block with empty transaction vector
Block::Block(blockID_t id, uint64_t height, blockID_t parentID, 
            time_t timestamp, minerID_t owner)
    : id(id), height(height), parentID(parentID), 
      timestamp(timestamp), owner(owner) {}

// Copy constructor - performs deep copy of all block members
Block::Block(const Block& block)
    : id(block.id), height(block.height), parentID(block.parentID), 
      transactions(block.transactions), timestamp(block.timestamp), owner(block.owner) {}

// Assignment operator - copies all members from the source block
Block& Block::operator=(const Block& block) {
    this->id = block.id;
    this->height = block.height;
    this->parentID = block.parentID;
    this->transactions = block.transactions;
    this->timestamp = block.timestamp;
    this->owner = block.owner;
    return *this;
}

// Equality operator - compares all members except owner
// Two blocks are considered equal if they have the same id, height, parentID,
// transactions list, and timestamp
bool Block::operator==(const Block& block) const {
    return this->id == block.id && 
           this->height == block.height && 
           this->parentID == block.parentID && 
           this->transactions == block.transactions && 
           this->timestamp == block.timestamp;
}

// Inequality operator - implemented using equality operator
bool Block::operator!=(const Block& block) const {
    return !(*this == block);
}

// Less than operator - compares blocks based on their IDs only
// Used for ordering blocks in containers
bool Block::operator<(const Block& block) const {
    return this->id < block.id;
}

// Calculate the total size of transactions in the block
// Returns size in bytes (number of transactions * Kb * 8)
size_t Block::dataSize() const {
    return transactions.size() * Kb * 8;
}

hash_t Block::hash() const {
    hash_t hash_id = std::hash<blockID_t>{}(id);
    hash_t hash_height = std::hash<uint64_t>{}(height);
    hash_t hash_parentID = std::hash<blockID_t>{}(parentID);
    hash_t hash_timestamp = std::hash<time_t>{}(timestamp);
    hash_t hash_owner = std::hash<minerID_t>{}(owner);
    hash_t hash_transactions = std::reduce(transactions.begin(), transactions.end(), hash_t(0), [](hash_t acc, Transaction t) {
        return acc ^ t.hash();
    });
    return hash_id ^ hash_height ^ hash_parentID ^ hash_timestamp ^ hash_owner ^ hash_transactions;
}