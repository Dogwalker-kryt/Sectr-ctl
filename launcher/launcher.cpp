#include <iostream>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <pwd.h>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "../DriveMgr_CLI/include/EnvSys.hpp" // this is the include path for the EnvSys.hpp, of my Project fs strcutue

std::filesystem::path dmgr_root = EnvSys::appRoot();
std::filesystem::path bin_path = dmgr_root / "bin" / "bin" / "ox"; 
std::filesystem::path config_path = dmgr_root / "data" / "config.conf";

struct CONFIG_VALUES {
    std::string UI_MODE = "CLI";
    std::string COMPILE_MODE = "StatBin";
    bool ROOT_MODE = false;
    bool DRY_RUN_MODE = false;
};

std::unordered_map<std::string, std::filesystem::path> Programm_locations = {
    {"CLI", bin_path},
};


/**
 * @brief Read and parse configuration file
 * @return CONFIG_VALUES with parsed settings, or defaults if file doesn't exist
 */
CONFIG_VALUES configHandler() {
    CONFIG_VALUES cfg{};

    if (!std::filesystem::exists(config_path)) {
        std::cerr << "[Config_handler WARNING] Config not found: " << config_path << ", using defaults\n";
        return cfg;  // Return defaults instead of silent failure
    }

    std::ifstream config_file(config_path);
    
    if (!config_file.is_open()) {
        std::cerr << "[Config_handler ERROR] Cannot open config file: " << config_path << "\n";
        return cfg;
    }

    std::string line;
    while (std::getline(config_file, line)) {

        if (line.empty() || line[0] == '#')
            continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos)
            continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);

        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        if (key == "UI_MODE") cfg.UI_MODE = value;
        else if (key == "COMPILE_MODE") cfg.COMPILE_MODE = value;
        else if (key == "DRY_RUN_MODE") {
            std::string v = value;
            std::transform(v.begin(), v.end(), v.begin(), ::tolower);
            cfg.DRY_RUN_MODE = (v == "true");
        }
        else if (key == "ROOT_MODE") {
            std::string v = value;
            std::transform(v.begin(), v.end(), v.begin(), ::tolower);
            cfg.ROOT_MODE = (v == "true");
        }
    }

    config_file.close();
    return cfg;
}

/**
 * @brief Validate that binary exists and is executable
 * @param program_path Path to the binary
 * @return true if valid, false otherwise
 */
bool validateBinary(const std::filesystem::path &program_path) {
    if (!std::filesystem::exists(program_path)) {
        std::cerr << "[Launcher ERROR] Binary not found: " << program_path << "\n";
        return false;
    }

    // Check if executable
    if (access(program_path.c_str(), X_OK) != 0) {
        std::cerr << "[Launcher ERROR] Binary not executable: " << program_path << "\n";
        return false;
    }

    return true;
}

/**
 * @brief Launch program using fork + exec (safer than system())
 * @param program_path Path to the binary
 * @param root Whether to run with sudo
 * @param dry_run Whether to add --dry-run flag
 * @param program_flags Additional flags to pass
 */
void launch_program(const std::filesystem::path &program_path, bool root, bool dry_run, const std::string &program_flags = "") {
    
    if (!validateBinary(program_path)) {
        exit(1);
    }

    // Build argument list
    std::vector<const char*> args;
    std::string prog_str = program_path.string();

    if (root) {
        args.push_back("sudo");
        args.push_back(prog_str.c_str());
        
    } else {
        args.push_back(prog_str.c_str());
    }

    if (dry_run) {
        args.push_back("--dry-run");
    }

    if (!program_flags.empty()) {
        args.push_back(program_flags.c_str());
    }

    args.push_back(nullptr);  // NULL-terminate for execvp

    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "[Launcher ERROR] fork() failed\n";
        exit(1);
    }

    if (pid == 0) {
        // Child process: execute
        execvp(args[0], (char* const*)args.data());

        // If execvp returns, it failed
        std::cerr << "[Launcher ERROR] execvp() failed: " << strerror(errno) << "\n";
        exit(1);

    } else {
        // Parent process: wait for child
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {

            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {

                std::cerr << "[Launcher WARNING] Child exited with code: " << exit_code << "\n";
            }
            exit(exit_code);

        } else if (WIFSIGNALED(status)) {

            std::cerr << "[Launcher ERROR] Child killed by signal: " << WTERMSIG(status) << "\n";
            exit(1);

        }
    }
}


int main(int argc, char* argv[]) {

    CONFIG_VALUES cfg = configHandler();

    std::string flag;
    if (argc > 1) {
        flag = argv[1];
    }

    // Validate UI_MODE exists in map
    std::string selected_ui = cfg.UI_MODE;
    if (Programm_locations.find(selected_ui) == Programm_locations.end()) {
        std::cerr << "[Launcher ERROR] Unknown UI_MODE: " << selected_ui << ", defaulting to CLI\n";
        selected_ui = "CLI";
    }

    std::filesystem::path program_to_launch = Programm_locations[selected_ui];
    
    std::cout << "[Launcher] Starting " << selected_ui << " from: " << program_to_launch << "\n";

    launch_program(program_to_launch, cfg.ROOT_MODE, cfg.DRY_RUN_MODE, flag);
    
    return 0;
}

