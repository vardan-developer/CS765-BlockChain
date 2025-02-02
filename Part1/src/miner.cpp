#include "miner.hpp"

Miner::Miner(int id, double hashPower, std::vector<minerId_t> neighbours)
{
    this->id = id;
    this->hashPower = hashPower;
    this->blockTree = BlockTree();
    this->currentBlock = Block();
    this->currentHeight = 0;
    this->amount = 0;
    this->currentScheduledBlock = nullptr;
    this->unspentUtxos = std::queue<Utxo>();
    this->neighbours = neighbours;
}

std::vector<Event> Miner::receiveEvent(Event &event)
{
    std::vector<Event> newEvents;
    switch (event.type)
    {
    case EventType::RECEIVE_BROADCAST_TRANSACTION:
        return receiveBroadcastTransaction(event);
    case EventType::RECEIVE_BROADCAST_BLOCK:
        return receiveBroadcastBlock(event);
    case EventType::BROADCAST_BLOCK:
        return generateBlock(event.timestamp);
    case EventType::BROADCAST_TRANSACTION:
        return generateTransaction(event.timestamp);
    case EventType::BLOCK_CREATION:
        return confirmBlock(event);
    default:
        break;
    }
}

std::vector<Event> Miner::confirmBlock(Event &event)
{
    int height = blockTree.getCurrentHeight();
    if (!currentScheduledBlock || *currentScheduledBlock != *event.block || event.block->height <= height) // TODO: implement != operator for block
    {
        return std::vector<Event>();
    }
    currentBlock = *event.block;
    for(auto txn : currentBlock.transactions){
        if(memPool.find(txn) != memPool.end()){
            memPool.erase(txn);
        }

    }
    blockTree.addBlock(*event.block, event.timestamp);
    return std::vector<Event>{event};
}

std::vector<Event> Miner::generateBlock(time_t prev_time)
{
    currentScheduledBlock = nullptr;
    time_t scheduleTime = prev_time + getExponentialRandom(BLOCK_INTER_ARRIVAL_TIME / hashPower);

    blockId_t scheduledBlockID = Counter::getBlockID();
    txnId_t coinBaseTxnID = Counter::getTxnID();
    currentScheduledBlock = new Block(scheduledBlockID, currentHeight + 1, currentBlock.id, scheduleTime);

    Transaction coinbase = Transaction(coinBaseTxnID, std::vector<Utxo>(), std::vector<Utxo>{Utxo(scheduledBlockID, coinBaseTxnID, 0, id, MINING_REWARD)}, TransactionType::COINBASE);

    currentScheduledBlock->transactions.push_back(coinbase);

    if (memPool.size() == 0)
    {
        return std::vector<Event>{Event(EventType::BROADCAST_BLOCK, currentScheduledBlock, scheduleTime, id)};
    }

    // Add random transactions to the block
    int num_txns = getUniformRandom(1, std::min((int)memPool.size(), 100));
    for(int i = 0; i < num_txns; i++){
        currentScheduledBlock->transactions.push_back(*(memPool.begin()));
        for(Utxo& utxo : currentScheduledBlock->transactions.back().out_utxos){
            utxo.block = scheduledBlockID;
        }
        memPool.erase(memPool.begin());
    }

    return std::vector<Event>{Event(EventType::BROADCAST_BLOCK, currentScheduledBlock, scheduleTime, id)};
}

std::vector<Event> Miner::generateTransaction(time_t prev_time)
{
    if(amount == 0){
        return std::vector<Event>();
    }
    time_t scheduleTime = prev_time + getExponentialRandom(TXN_INTER_ARRIVAL_TIME);
    txnId_t txnID = Counter::getTxnID();
    std::vector<Utxo> in_utxos;
    std::vector<Utxo> out_utxos;

    int paymentAmount = getUniformRandom(1, amount);
    minerId_t paymentReceiver;

    while((paymentReceiver = getUniformRandom(1, NUM_MINERS)) == id);

    int currentAmount = 0;
    
    while(currentAmount < paymentAmount){
        
        for (int i = 0; i < unspentUtxos.size() and !blockTree.verifyUtxo(unspentUtxos.front()); i++) { // TODO: implement verifyUtxo function
            unspentUtxos.push(unspentUtxos.front());
            unspentUtxos.pop();
        }
        
        in_utxos.push_back(unspentUtxos.front());
        currentAmount += unspentUtxos.front().amount;
        unspentUtxos.pop();
    }
    amount -= currentAmount;
    out_utxos.push_back(Utxo(-1, txnID, 0, paymentReceiver, paymentAmount));
    if(currentAmount > paymentAmount){
        out_utxos.push_back(Utxo(-1, txnID, 1, id, currentAmount - paymentAmount));
    }

    Transaction txn = Transaction(txnID, in_utxos, out_utxos, TransactionType::NORMAL);
    std::vector<Event> newEvents;
    newEvents.push_back(Event(EventType::BROADCAST_TRANSACTION, &txn, scheduleTime, id));
    return newEvents;
}

std::vector<Event> Miner::receiveBroadcastBlock(Event &event)
{
    std::vector<Event> newEvents;
    int sendingMiner = event.owner;
    blockToMiners[event.block->id].insert(sendingMiner);

    if(blockTree.addBlock(*event.block, event.timestamp) < 0){
        return newEvents;
    }

    if(blockTree.getCurrentHeight() > currentHeight){
        currentScheduledBlock = nullptr;
        currentBlock = *event.block;
        currentHeight = blockTree.getCurrentHeight();
        newEvents.push_back(generateBlock(event.timestamp)[0]);
    }
    for(auto peer: neighbours){
        if(blockToMiners[event.block->id].find(peer) == blockToMiners[event.block->id].end()){
            blockToMiners[event.block->id].insert(peer);
            newEvents.push_back(Event(EventType::SEND_BROADCAST_BLOCK, event.block, event.timestamp, id));
        }
    }
    return newEvents;
}

