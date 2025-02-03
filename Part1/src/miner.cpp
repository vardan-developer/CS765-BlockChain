#include "miner.hpp"

Miner::Miner(int id, double hashPower, std::vector<minerId_t> neighbours)
{
    this->id = id;
    this->hashPower = hashPower;
    this->blockTree = BlockTree(id);
    this->currentBlock = Block();
    this->currentHeight = 0;
    this->currentScheduledBlock = nullptr;
    this->neighbours = neighbours;
    this->currentScheduledTransactionTime = 0;
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
    case EventType::BLOCK_CREATION:
        return confirmBlock(event);
    default:
        return std::vector<Event>();
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
    blockTree.addBlock(*event.block, event.timestamp, memPool);
    return std::vector<Event>{event};
}

std::vector<Event> Miner::generateBlock(time_t prev_time)
{
    if ( currentScheduledBlock != nullptr )
    {
        return std::vector<Event>();
    }

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
    if (prev_time < currentScheduledTransactionTime)
    {
        return std::vector<Event>();
    }
    if(blockTree.getBalance() <= 0){
        return std::vector<Event>();
    }
    time_t scheduleTime = prev_time + getExponentialRandom(TXN_INTER_ARRIVAL_TIME);
    txnId_t txnID = Counter::getTxnID();

    int paymentAmount = getUniformRandom(1, blockTree.getBalance());
    minerId_t paymentReceiver;

    while((paymentReceiver = getUniformRandom(1, NUM_MINERS)) == id);

    int change;
    std::vector<Utxo> in_utxos = blockTree.getUtxos(paymentAmount, change);
    if ( in_utxos.empty() ) {
        return std::vector<Event>();
    }
    std::vector<Utxo> out_utxos;

    out_utxos.push_back(Utxo(-1, txnID, 0, paymentReceiver, paymentAmount));
    if(change > 0){
        out_utxos.push_back(Utxo(-1, txnID, 1, id, change));
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

    if(blockTree.addBlock(*event.block, event.timestamp, memPool) < 0){
        return newEvents;
    }

    if(blockTree.getCurrentHeight() > currentHeight){
        // TODO: Add back transactions to mempool
        for (auto txn: currentScheduledBlock->transactions){
            memPool.insert(txn);
        }
        currentScheduledBlock = nullptr;
        currentBlock = *event.block;
        currentHeight = blockTree.getCurrentHeight();
        std::vector<Event> newEvents = generateBlock(event.timestamp);
        newEvents.insert(newEvents.end(), newEvents.begin(), newEvents.end());
    }
    for(auto peer: neighbours){
        if(blockToMiners[event.block->id].find(peer) == blockToMiners[event.block->id].end()){
            blockToMiners[event.block->id].insert(peer);
            newEvents.push_back(Event(EventType::SEND_BROADCAST_BLOCK, event.block, event.timestamp, id));
        }
    }
    return newEvents;
}


std::vector<Event> Miner::receiveBroadcastTransaction(Event &event){
    memPool.insert(*event.transaction);
    blockToTransactions[event.block->id].insert(event.transaction->id);

    std::vector<Event> newEvents;

    for (auto peer: neighbours){
        if(blockToMiners[event.block->id].find(peer) == blockToMiners[event.block->id].end()){
            blockToMiners[event.block->id].insert(peer);
            newEvents.push_back(Event(EventType::SEND_BROADCAST_TRANSACTION, event.transaction, event.timestamp, id));
        }
    }

    return newEvents;
}

void Miner::addEvent(Event &event){
    eventList.push_back(event);
}

std::vector<Event> Miner::getEventList(time_t timestamp){
    std::vector<Event> newEvents = generateBlock(timestamp);
    eventList.insert(eventList.end(), newEvents.begin(), newEvents.end());
    newEvents = generateTransaction(timestamp);
    eventList.insert(eventList.end(), newEvents.begin(), newEvents.end());
    return eventList;
}