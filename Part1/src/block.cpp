#include "block.hpp"

Block::Block(): id(0), height(0), parentID(0), transactions(), timestamp(0) {}

Block::Block(blockID_t id, uint64_t height, blockID_t parentID, std::vector<Transaction> transactions, time_t timestamp): id(id), height(height), parentID(parentID), transactions(transactions), timestamp(timestamp) {}

Block::Block(blockID_t id, uint64_t height, blockID_t parentID, time_t timestamp): id(id), height(height), parentID(parentID), timestamp(timestamp) {}

Block::Block(const Block& block): id(block.id), height(block.height), parentID(block.parentID), transactions(block.transactions), timestamp(block.timestamp) {}

Block& Block::operator=(const Block& block){
    this->id = block.id;
    this->height = block.height;
    this->parentID = block.parentID;
    this->transactions = block.transactions;
    this->timestamp = block.timestamp;
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
    return sizeof(Block) + std::accumulate(transactions.begin(), transactions.end(), (size_t) 0, [](size_t sum, Transaction txn) { return sum + txn.dataSize(); });
}