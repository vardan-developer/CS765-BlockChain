#ifndef SIM_HPP
#define SIM_HPP

#include "miner.hpp"


class Simulator{
private:
    std::vector<Miner> miners;
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> events;
    time_t currentTime;
    int totalMiners;
    int txnInterval;
    int blkInterval;
    int limit;
    Block createGenesisBlock();
    
public:
    Simulator(int n, int txnInterval, int blkInterval, int limit = 5000);
    void run();
    void getEvents();
    void addEvent(Event event);
    void addEvents(std::vector<Event> events);
    void processEvent(Event event);

};

#endif
