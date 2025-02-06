#include "parser.hpp"
#include <climits>
#include <ctime>


bool parseArgs(int argc, char* argv[], ProgramSettings &settings, std::string &errorMsg) {
    bool totalNodes_set = false, z0_set = false, z1_set = false, Ttx_set = false, I_set = false, blkLimit_set = false, timeLimit_set = false;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--total-nodes") {
            if (i + 1 >= argc) {
                errorMsg = "Error: Missing value for --total-nodes";
                return false;
            }
            try {
                settings.totalNodes = std::stoi(argv[++i]);
                totalNodes_set = true;
            } catch (const std::exception &e) {
                errorMsg = "Error: Invalid integer for --total-nodes";
                return false;
            }
        } else if (arg == "--slow") {
            if (i + 1 >= argc) {
                errorMsg = "Error: Missing value for --slow";
                return false;
            }
            try {
                settings.z0 = std::stof(argv[++i]);
                z0_set = true;
            } catch (const std::exception &e) {
                errorMsg = "Error: Invalid float for --slow (z0)";
                return false;
            }
        } else if (arg == "--low-cpu") {
            if (i + 1 >= argc) {
                errorMsg = "Error: Missing value for --low-cpu";
                return false;
            }
            try {
                settings.z1 = std::stof(argv[++i]);
                z1_set = true;
            } catch (const std::exception &e) {
                errorMsg = "Error: Invalid float for --low-cpu (z1)";
                return false;
            }
        } else if (arg == "--ttx-time") {
            if (i + 1 >= argc) {
                errorMsg = "Error: Missing value for --ttx-time";
                return false;
            }
            try {
                settings.Ttx = std::stod(argv[++i]);
                Ttx_set = true;
            } catch (const std::exception &e) {
                errorMsg = "Error: Invalid double for --ttx-time";
                return false;
            }
        } else if (arg == "--blk-time") {
            if (i + 1 >= argc) {
                errorMsg = "Error: Missing value for --blk-time";
                return false;
            }
            try {
                settings.I = std::stoi(argv[++i]);
                I_set = true;
            } catch (const std::exception &e) {
                errorMsg = "Error: Invalid integer for --blk-time";
                return false;
            }
        } else if (arg == "--blk-limit") {
            if (i + 1 >= argc) {
                errorMsg = "Error: Missing value for --blk-limit";
                return false;
            }
            try {
                settings.blkLimit = std::stoi(argv[++i]);
                blkLimit_set = true;
            } catch (const std::exception &e) {
                errorMsg = "Error: Invalid integer for --blk-limit";
                return false;
            }
        } else if (arg == "--time-limit") {
            if (i + 1 >= argc) {
                errorMsg = "Error: Missing value for --time-limit";
                return false;
            }
            try {
                settings.timeLimit = std::stoi(argv[++i]);
                timeLimit_set = true;
            } catch (const std::exception &e) {
                errorMsg = "Error: Invalid integer for --time-limit";
                return false;
            }
        } else {
            errorMsg = "Error: Unknown parameter: " + arg;
            return false;
        }
    }
    
    // Check that all required parameters were provided.
    if (!totalNodes_set || !z0_set || !z1_set || !Ttx_set || !I_set || ( !blkLimit_set && !timeLimit_set) ) {
        errorMsg = "Error: One or more required parameters are missing.";
        return false;
    }

    if ( ! timeLimit_set ) settings.timeLimit = LONG_LONG_MAX;
    if ( ! blkLimit_set ) settings.blkLimit = LONG_LONG_MAX;
    
    // Validate that z0 and z1 are between 0 and 1.
    if (settings.z0 < 0.0f || settings.z0 > 1.0f) {
        errorMsg = "Error: --slow (z0) must be a float between 0 and 1.";
        return false;
    }
    if (settings.z1 < 0.0f || settings.z1 > 1.0f) {
        errorMsg = "Error: --low-cpu (z1) must be a float between 0 and 1.";
        return false;
    }
    
    return true;
} 