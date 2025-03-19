#include "block.hpp"

Block::Block(): id(0), height(0), parentID(0), transactions(), timestamp(0), owner(-1) {}

Block::Block(blockID_t id, uint64_t height, blockID_t parentID, std::vector<Transaction> transactions, time_t timestamp, minerID_t owner): id(id), height(height), parentID(parentID), transactions(transactions), timestamp(timestamp), owner(owner) {}

Block::Block(blockID_t id, uint64_t height, blockID_t parentID, time_t timestamp, minerID_t owner): id(id), height(height), parentID(parentID), timestamp(timestamp), owner(owner) {}

Block::Block(const Block& block): id(block.id), height(block.height), parentID(block.parentID), transactions(block.transactions), timestamp(block.timestamp), owner(block.owner) {}

Block& Block::operator=(const Block& block){
    this->id = block.id;
    this->height = block.height;
    this->parentID = block.parentID;
    this->transactions = block.transactions;
    this->timestamp = block.timestamp;
    this->owner = block.owner;
    return *this;
}

bool Block::operator==(const Block& block) const{
    return this->id == block.id && this->height == block.height && this->parentID == block.parentID && this->transactions == block.transactions && this->timestamp == block.timestamp;
}

bool Block::operator!=(const Block& block) const{
    return !(*this == block);
}

bool Block::operator<(const Block& block) const{
    return this->id < block.id;
}

size_t Block::dataSize() const {
    return  transactions.size() * Kb * 8;
}