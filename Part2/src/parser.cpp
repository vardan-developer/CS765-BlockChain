#include "parser.hpp"
#include <climits>
#include <ctime>
#include "def.hpp"

bool parseArgs(int argc, char* argv[], ProgramSettings &settings, std::string &errorMsg) {
    bool totalNodes_set = false, Ttx_set = false, I_set = false, blkLimit_set = false, timeLimit_set = false;
    bool Tt_set = false, eclipse_set = false, malicious_set = false;
    
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
        // Removed z0 and z1 parameter handling
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
        } else if (arg == "--timeout") {
            if (i + 1 >= argc) {
                errorMsg = "Error: Missing value for --timeout";
                return false;
            }
            try {
                settings.Tt = std::stod(argv[++i]);
                Tt_set = true;
            } catch (const std::exception &e) {
                errorMsg = "Error: Invalid double for --timeout";
                return false;
            }
        } else if (arg == "--eclipse") {
            if (i + 1 >= argc) {
                errorMsg = "Error: Missing value for --eclipse";
                return false;
            }
            std::string value = argv[++i];
            if (value == "true" || value == "1") {
                settings.eclipse = true;
                eclipse_set = true;
            } else if (value == "false" || value == "0") {
                settings.eclipse = false;
                eclipse_set = true;
            } else {
                errorMsg = "Error: Invalid boolean for --eclipse (use 'true'/'false' or '1'/'0')";
                return false;
            }
        } else if (arg == "--malicious") {
            if (i + 1 >= argc) {
                errorMsg = "Error: Missing value for --malicious";
                return false;
            }
            try {
                settings.malicious = std::stod(argv[++i]);
                malicious_set = true;
                if (settings.malicious < 0.0 || settings.malicious > 1.0) {
                    errorMsg = "Error: --malicious must be a float between 0 and 1.";
                    return false;
                }
            } catch (const std::exception &e) {
                errorMsg = "Error: Invalid double for --malicious";
                return false;
            }
        } else {
            errorMsg = "Error: Unknown parameter: " + arg;
            return false;
        }
    }
    
    // Check that all required parameters were provided.
    if (!totalNodes_set || !Ttx_set || !I_set || !Tt_set || !malicious_set || (!blkLimit_set && !timeLimit_set)) {
        errorMsg = "Error: One or more required parameters are missing.";
        return false;
    }

    // Set default values for optional parameters
    if (!timeLimit_set) settings.timeLimit = LONG_LONG_MAX_VAL;
    if (!blkLimit_set) settings.blkLimit = LONG_LONG_MAX_VAL;
    if (!eclipse_set) settings.eclipse = false; // Default to no eclipse attack
    
    return true;
}