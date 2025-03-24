#include "transaction.hpp"

Transaction::Transaction(txnID_t id, TransactionType type, minerID_t sender, minerID_t receiver, uint32_t amount){
    this->id = id;
    this->type = type;
    this->sender = sender;
    this->receiver = receiver;
    this->amount = amount;
}

hash_t Transaction::hash() const {
    hash_t hash_id = std::hash<txnID_t>{}(id);
    hash_t hash_type = std::hash<TransactionType>{}(type);
    hash_t hash_sender = std::hash<minerID_t>{}(sender);
    hash_t hash_receiver = std::hash<minerID_t>{}(receiver);
    hash_t hash_amount = std::hash<uint32_t>{}(amount);
    return hash_id ^ hash_type ^ hash_sender ^ hash_receiver ^ hash_amount;
}