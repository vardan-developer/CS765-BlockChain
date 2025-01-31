#include "block.hpp"

Block::Block() {
    id = 0;
    height = 0;
    parent_id = 0;
    transactions = std::vector<Transaction>();
    timestamp = 0;
}

Block::Block(const Block & other) {
    this->id = other.id;
    this->height = other.height;
    this->parent_id = other.parent_id;
    this->transactions = other.transactions;
    this->timestamp = other.timestamp;
}

Block & Block::operator=(const Block & other) {
    this->id = other.id;
    this->height = other.height;
    this->parent_id = other.parent_id;
    this->transactions = other.transactions;
    this->timestamp = other.timestamp;
    return *this;
}

bool Block::operator < (const Block& other) const {
    return id < other.id;
}