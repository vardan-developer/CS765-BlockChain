#ifndef SIM_HPP
#define SIM_HPP

#include "miner.hpp"
#include <ctime>


class Simulator{
private:
    std::vector<Miner *> miners;
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> events;
    time_t currentTime;
    int totalMiners;
    int txnInterval;
    int blkInterval;
    time_t timeLimit;
    Block createGenesisBlock();
    
public:
    Simulator(int n, int txnInterval, int blkInterval, time_t timeLimit = 50000);
    ~Simulator();
    void run();
    void getEvents();
    void addEvent(Event event);
    void addEvents(std::vector<Event> events);
    void processEvent(Event event);

};

#endif
