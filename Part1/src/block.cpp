#include "block.hpp"

Block::Block(){
    this->id = 0;
    this->height = 0;
    this->parentID = 0;
    this->transactions = std::vector<Transaction>();
    this->timestamp = 0;
}

Block::Block(blockID_t id, uint64_t height, blockID_t parentID, std::vector<Transaction> transactions, time_t timestamp){
    this->id = id;
    this->height = height;
    this->parentID = parentID;
    this->transactions = transactions;
    this->timestamp = timestamp;
}

Block::Block(blockID_t id, uint64_t height, blockID_t parentID, time_t timestamp){
    this->id = id;
    this->height = height;
    this->parentID = parentID;
    this->transactions = std::vector<Transaction>();
    this->timestamp = timestamp;
}

Block::Block(const Block& block){
    this->id = block.id;
    this->height = block.height;
    this->parentID = block.parentID;
    this->transactions = block.transactions;
    this->timestamp = block.timestamp;
}

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
    return sizeof(Block) + std::accumulate(transactions.begin(), transactions.end(), (size_t) 0, [](size_t sum, Transaction & txn) { return sum + txn.dataSize(); });
}