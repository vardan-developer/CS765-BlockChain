#include <"transaction.hpp">

Transaction::Transaction(txnID_t id, TransactionType type, minerID_t sender, minerID_t receiver, uint32_t amount){
    this->id = id;
    this->type = type;
    this->sender = sender;
    this->receiver = receiver;
    this->amount = amount;
}