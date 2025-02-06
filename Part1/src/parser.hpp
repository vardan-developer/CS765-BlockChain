#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <istream>
#include <string>
#include <sstream>


// Structure to hold program parameters.
struct ProgramSettings {
    int totalNodes;   // Total number of nodes
    float z0;         // A float between 0 and 1
    float z1;         // A float between 0 and 1
    double Ttx;       // A floating point value (Ttx)
    int I;            // An integer argument I
    long long blkLimit;
    time_t timeLimit;
};

// Parses command line arguments.
// Expected flags and following values:
//   --total-nodes [int]
//   --slow [float]    (for z0)
//   --low-cpu [float] (for z1)
//   --ttx-time [double]
//   --blk-time [int]
// On success, returns true and fills in 'settings'.
// On failure, returns false and 'errorMsg' contains a description of the error.
bool parseArgs(int argc, char* argv[], ProgramSettings &settings, std::string &errorMsg);

#endif // PARSER_HPP 