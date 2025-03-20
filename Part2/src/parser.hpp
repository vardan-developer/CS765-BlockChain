#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <istream>
#include <string>
#include <sstream>


// Structure to hold program parameters.
struct ProgramSettings {
    int totalNodes;   // Total number of nodes
    double Ttx;       // A floating point value (Ttx)
    long long I;            // An integer argument I
    double Tt;         // Timeout Parameter
    bool eclipse;       // Enable eclipse attack
    double malicious;   // Fraction of malicious nodes
    long long blkLimit;
    time_t timeLimit;
};

// Parses command line arguments.
// Expected flags and following values:
//   --total-nodes [int]     Total number of nodes in the network
//   --ttx-time [double]     Transaction propagation time
//   --blk-time [int]        Block interval time
//   --blk-limit [long]      Maximum number of blocks to simulate
//   --time-limit [time_t]   Maximum simulation time
//   --timeout [double]      Timeout parameter
//   --eclipse [bool]        Enable eclipse attack (true/false)
//   --malicious [double]    Fraction of malicious nodes (0.0-1.0)
// On success, returns true and fills in 'settings'.
// On failure, returns false and 'errorMsg' contains a description of the error.
bool parseArgs(int argc, char* argv[], ProgramSettings &settings, std::string &errorMsg);

#endif // PARSER_HPP 