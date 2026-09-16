#include "../include/DmgrLib.h"

// ========= Logger =========

enum class LogType {
    ERROR,
    WARNING,
    INFO,
    SUCCESS,
    DRYRUN,
    EXEC
};

const char* Logger::logMessage(LogType log_type) {
    switch (log_type) {
        case LogType::ERROR: return err_s;
        case LogType::WARNING: return warn_s;
        case LogType::INFO: return info_s;
        case LogType::SUCCESS: return success_s;
        case LogType::DRYRUN: return dryrun_s;
        case LogType::EXEC: return exec_s;
        default: return unknown_s;
    }
}

void Logger::log(LogType type, const scf::str1024 &operation, const char* func) {
        if (Globals::g_no_log == false) {

            auto now = std::chrono::system_clock::now();
            std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
            char timeStr[100];

            std::strftime(timeStr, sizeof(timeStr), "%d-%m-%Y %H:%M", std::localtime(&currentTime));

            scf::str2048 log_msg = "[" + scf::to_str64(timeStr) + "] event: " + scf::to_str16(logMessage(type)) + operation + " (location: " + scf::to_str32(func) + ")";

            std::ofstream log_file(Globals::log_path, std::ios::app);

            if (log_file) {

                log_file << log_msg << std::endl;

            } else {
                std::cerr << RED << "[Logger Error] Unable to open log file: " << Globals::log_path << " Reason: " << strerror(errno) << RESET <<"\n";
            }

        } else {
            return;
        }
    }

void Logger::error(const scf::str1024 &msg, const char* func) {
    log(LogType::ERROR, msg, func);
}

void Logger::warning(const scf::str1024 &msg, const char* func) {
    log(LogType::WARNING, msg, func);
}

void Logger::info(const scf::str1024 &msg, const char* func) {
    log(LogType::INFO, msg, func);
}

void Logger::success(const scf::str1024 &msg, const char* func) {
    log(LogType::SUCCESS, msg, func);
}

void Logger::dry_run(const scf::str1024 &msg, const char* func) {
    log(LogType::DRYRUN, msg, func);
}

void Logger::exec(const scf::str1024 &msg, const char* func) {
    log(LogType::EXEC, msg, func);
}

bool Logger::clearLoggs(const char *path) {
    FILE *log_file = fopen(path, "w");
    if (log_file == nullptr) {
        return false;
    }
    fclose(log_file);
    return true;
}


// ========= helper/validtion/runtime error =========

// file_path string format is always: /path/file.extension
const scf::str1024 filePathHandler(const scf::str<986> &file_path) {    
    const char* sudo_user = getenv("SUDO_USER");
    const char* user_env = getenv("USER");
    const char* username = sudo_user ? sudo_user : user_env;

    if (!username) {
        scf::println_cerr(RED, "[LOG_ERROR] Could not determine username.", RESET);
        LOG_ERROR("Could not determine username");
        return "";
    }

    const struct passwd *pw = getpwnam(username);

    if (!pw) {
        scf::println_cerr(RED, "[LOG_ERROR] Could not get home directory for user: ", username, RESET);
        LOG_ERROR("Failed to get home directory for user: " + scf::to_str64(username));
        return "";
    }

    scf::str_t homeDir = pw->pw_dir;
    scf::str1024 path = homeDir + file_path;
    
    return path;
}


// ========= input validation =========
template<size_t N>
scf::str<N> readLine() {
    scf::str<N> str;

    if (!scf::read(str, N)) {
       ERR(ErrorCode::FailedInput, "Failed to read input");
       LOG_ERROR("scf::read<>() failed to get input");
       return "";
    }

    return str;
}

char *readLine_64() {
    static char s[64];
    fgets(s, sizeof(s), stdin);
    return s;
}

namespace InputValidation {

    scf::optional<int> getInt(const std::vector<int> &valid_ints) {
        const scf::str_t s_input = readLine<128>();

        if (s_input.empty()) {
            return scf::nullopt;
        }

        try {

            size_t idx = 0;
            const int i_input = std::stoi(s_input.c_str(), &idx);

            if (idx != s_input.size()) {

                ERR(ErrorCode::InvalidInput, "no characters can be used as input");
                LOG_ERROR("no characters can be used as input");
                return scf::nullopt;

            }

            if (!valid_ints.empty() && std::find(valid_ints.begin(), valid_ints.end(), i_input) == valid_ints.end()) {

                ERR(ErrorCode::InvalidInput, "Input not in allowed integer list");
                LOG_ERROR("Input not in allowed integer list -> validateIntInput");
                return scf::nullopt;

            }

            return i_input;

        } catch (const std::exception&) {

            ERR(ErrorCode::InvalidInput, "Conversion from string to int failed");
            LOG_ERROR("Conversion from string to int failed -> validateIntInput");
            return scf::nullopt;

        }
    }

    scf::optional<int> getInt(int min_value, int max_value) {
        const auto val = getInt({});
        if (!val) return scf::nullopt;

        if (*val < min_value || *val > max_value) {

            ERR(ErrorCode::InvalidInput, "Input outside allowed range");
            LOG_ERROR("Input outside allowed range");
            return scf::nullopt;

        }

        return val;
    }

    scf::optional<unsigned int> getUint() {
        scf::str256 s_input = readLine<256>();

        if (s_input.empty()) {
            return scf::nullopt;
        }

        try {

            size_t idx = 0;
            const unsigned long long tmp = std::stoull(s_input, &idx);

            if (idx != s_input.size()) {

                ERR(ErrorCode::InvalidInput, "only 1 character can be used as input");
                LOG_ERROR("no characters can be used as input");
                return scf::nullopt;

            }

            if (tmp > std::numeric_limits<unsigned int>::max()) {
                ERR(ErrorCode::OutOfRange, "Number too large for unsigned int");
                LOG_ERROR("Number too large for unsigned int");
                return scf::nullopt;
            }

            return static_cast<unsigned int>(tmp);

        } catch (const std::exception&) {

            ERR(ErrorCode::InvalidInput, "Conversion from string to uint failed");
            LOG_ERROR("Conversion from string to uint failed");
            return scf::nullopt;

        }
    }

    scf::optional<char> getChar(const std::vector<char> &valid_chars) {
        const scf::str8 input = readLine<8>();

        const std::string trimmed = StrUtils::trimWhiteSpace(scf::to_std_str(input));

        const char c_input = trimmed[0];        

        if (!valid_chars.empty() && std::find(valid_chars.begin(), valid_chars.end(), c_input) == valid_chars.end()) {

            ERR(ErrorCode::InvalidInput, "Character not allowed");
            LOG_ERROR("Char not in allowed list");
            return scf::nullopt;

        }

        return c_input;
    }

    scf::optional<std::string> getString(const uint8_t string_size) {
        uint8_t str_size = string_size;
        if (string_size > 64) str_size = 64;
        
        const char *s_input = readLine_64();

        if (!std::cin.good()) {

            ERR(ErrorCode::IOError, "Failed to read input");
            LOG_ERROR("std::getline() failed");
            return scf::nullopt;

        }

        std::string s = StrUtils::trimWhiteSpace(s_input);

        if (s.empty()) {

            ERR(ErrorCode::InvalidInput, "Input cannot be emtpy");
            LOG_ERROR("Input cannot be emtpy");
            return scf::nullopt;

        }

        if (s.size() > string_size && string_size > 0) {

            s = s.substr(0, string_size);
        
        }

        return s;
    }
}


// ==================== Side/Helper Functions ====================

const char *confirmationKeyGenerator() {
    constexpr char chars_for_key[62] = {
        'a','b','c','d','e','f','g','h','i','j',
        'k','l','m','n','o','p','q','r','s','t',
        'u','v','w','x','y','z',
        'A','B','C','D','E','F','G','H','I','J',
        'K','L','M','N','O','P','Q','R','S','T',
        'U','V','W','X','Y','Z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
    };

    static thread_local std::mt19937 gen(std::random_device{}());

    std::uniform_int_distribution<> dist(0, 62 - 1);

    static char generated_key[10 + 1] = {};

    for (int i = 0; i < 10; i++) {
        generated_key[i] = chars_for_key[dist(gen)];
    }

    generated_key[10] = '\0';

    return generated_key;
}

const bool askForConfirmation(const scf::str1024 &prompt) {
    scf::println(prompt, "(y/n)");
    auto confirm = InputValidation::getChar({'y', 'n'});
    if (!confirm.has_value()) return false;

    if (confirm != 'Y' && confirm != 'y') {
        std::cout << BOLD << "[INFO] Operation cancelled\n" << RESET;
        LOG_INFO("Operation cancelled");
        return false;
    } 

    return true;
}

void menuQues(bool& running) {   
    scf::lnprintln(BOLD, "Press any key to return to the main menu, '2' to exit:", RESET);

    auto menuques = InputValidation::getInt({1, 2});

    if (!menuques.has_value()) return;

    if (menuques == 1) {

        running = true;

    } else if (menuques == 2) {

        running = false;
    }
}

const bool isRoot() {
    return (getuid() == 0);
}

const bool checkRoot() {
    if (!isRoot()) {
        ERR(ErrorCode::PermissionDenied, "This function requires root privileges. Please run with 'sudo'");
        LOG_ERROR("Attempted to run without root privileges");
        return false;
    }
    return true;
}

const bool checkRootMetadata() {
    if (!isRoot()) {
        scf::println_cerr(YELLOW, "[WARNING] Running without root may limit functionality. For full access, please run with 'sudo'.\n", RESET);
        LOG_WARNING("Running without root privileges");
        return false;
    }
    return true;
}

void printFunctionHeader(const char* __s) {
    system("clear");
    scf::flush_stdout();
    scf::println_flush(BOLD, "[      ", __s, "      ]", RESET);
}

void cleanExit() {
    scf::print(LEAVETERMINALSCREEN);
    exit(1);
}

bool devSuffix() {
    return (Globals::version.rfind("v") != scf::npos);
}