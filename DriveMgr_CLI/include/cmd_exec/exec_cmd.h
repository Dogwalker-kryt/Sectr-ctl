/* 
 * DriveMgr - Linux Drive Management Utility
 * Copyright (C) 2025 Dogwalker-kryt
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include "../DmgrLib.h"
#include "command_exec.h"
#include "../ui/Spinner.hpp"
#include <atomic>

// ==================== Command Execution Abstraction ====================

enum class ExecMode {
    NORMAL,      // Regular execution
    DRY_RUN,     // Show what would run
    QUIET,       // No output to console, only logging
    PROGRESS     // Show spinner
};

// Extended result type for high-level usage
struct CmdExecResult {
    bool success;          // true if exit_code == 0
    std::string output;    // combined stdout + stderr
    int exit_code;         // raw exit code from the process
};

class CmdExec {
public:
    static inline CmdExecResult run(const char *cmd, bool use_sudo = false, ExecMode mode = ExecMode::NORMAL) {
        CmdExecResult result{false, "", -1};

        const size_t cmd_len = std::strlen(cmd);
        const char sudo_str[5] = { 's', 'u', 'd', 'o', ' '};
        char *final_cmd;
        if (use_sudo) {
            constexpr size_t sudo_len = std::strlen("sudo \0");

            final_cmd = (char *)malloc(cmd_len + sudo_len + 1);
            if (!final_cmd) {
                ERR(ErrorCode::AllocationFault, "[CMD_EXEC] buffer allocation for final_cmd failed");
                LOG_ERROR("[CMD_EXEC] buffer allocation for final_cmd failed");
                return result;
            }
            
            std::memcpy(final_cmd, "sudo ", sudo_len);
            std::memcpy(final_cmd + sudo_len, cmd, cmd_len);
            final_cmd[cmd_len + sudo_len] = '\0';

        } else {
            final_cmd = strdup(cmd);
        };

        if (Globals::g_dry_run || mode == ExecMode::DRY_RUN) {

            scf::println(YELLOW, "[DRY-RUN] Would execute: ", final_cmd, RESET);
            LOG_DRYRUN(final_cmd);

            result.success = true;
            result.exit_code = 0;
            return result;

        }

        std::atomic<bool> b_done = false;
        std::thread spinner;

        if (mode == ExecMode::PROGRESS) {
            spinner = std::thread([&]() {
                SPINNER(b_done);
            });
        }

        // Execute via low-level runner
        ExecResult r = run_command(final_cmd);
        free(final_cmd);
        b_done = true;
        
        // Spinner
        if (spinner.joinable()) {
            spinner.join();
        }
        
        // Fill high-level result
        result.exit_code = r.exit_code;
        result.success   = (r.exit_code == 0);

        // You can choose: only stdout, or stdout+stderr.
        // For drive ops, having both is usually better:
        result.output = r.stdout_str;
        if (!r.stderr_str.empty()) {

            if (!result.output.empty())
                result.output += "\n";

            result.output += r.stderr_str;
        }

        // Logging / console behavior
        if (mode == ExecMode::QUIET) {
            
            // scf::str1024 log_msg = scf::to_str512(cmd) + scf::to_str8(" -> ") + scf::to_str8(result.success ? "OK" : "FAILED");
            char log_msg[1024u];
            snprintf(log_msg, 1024u, "%s -> %s", cmd, result.success ? "OK" : "FAILED");
            LOG_EXEC(log_msg);

        } else {

            if (!result.output.empty()) {
                scf::println(result.output);
            }

        }
        return result;
    }

    static inline CmdExecResult run(const scf::str512 &cmd, bool use_sudo = false, ExecMode mode = ExecMode::NORMAL) {
        return run(cmd.c_str(), use_sudo, mode);
    }

    // Convenience overloads
    static inline CmdExecResult run_sudo(const std::string& cmd, ExecMode mode = ExecMode::NORMAL) {
        return run(cmd.c_str(), true, mode);
    }

    static inline CmdExecResult run_quiet(const std::string& cmd, bool use_sudo = false) {
        return run(cmd.c_str(), use_sudo, ExecMode::QUIET);
    }

    // Check and throw on failure
    static inline CmdExecResult run_or_throw(const std::string& cmd, bool use_sudo = false) {
        CmdExecResult res = run(cmd.c_str(), use_sudo);
        if (!res.success) {

            std::cout << LEAVETERMINALSCREEN;
            throw std::runtime_error("Command failed: " + cmd + "\nExit code: " + std::to_string(res.exit_code) + "\nOutput:\n" + res.output);
            
        }
        return res;
    }

    static inline CmdExecResult run_spinner(const scf::str512 &cmd) {
        return run(cmd.c_str(), false, ExecMode::PROGRESS);
    }

    static inline CmdExecResult run_sudo_spinner(const scf::str512 &cmd) {
        return run(cmd.c_str(), true, ExecMode::PROGRESS);
    }
};

// Quick helpers for common patterns
/** 
 * @brief runs ``` cmd ``` command
 * @param cmd as scf::str1024 
 * @returns res.output, res.success, res.exit_code
 * @note 
 * auto res = EXEC(cmd);
 * 
 * or
 *
 * EXEC(cmd)
 */
#define EXEC(cmd)              CmdExec::run(cmd)

/** 
 * @brief runs ``` cmd ``` command as Sudo user
 * @param cmd as scf::str1024 
 * @returns res.output, res.success, res.exit_code
 * @note 
 * auto res = EXEC_SUDO(cmd);
 */
#define EXEC_SUDO(cmd)         CmdExec::run_sudo(cmd)

/** 
 * @brief runs ``` cmd ``` command with no text ouput printed to the terminal
 * @param cmd as scf::str1024 
 * @returns res.output, res.success, res.exit_code
 * @note 
 * auto res = EXEC_QUIET(cmd);
 */
#define EXEC_QUIET(cmd)        CmdExec::run_quiet(cmd, false)

/** 
 * @brief runs ``` cmd ``` command as Sudo with no text ouput printed to the terminal
 * @param cmd as scf::str1024 
 * @returns res.output, res.success, res.exit_code
 * @note 
 * auto res = EXEC_QUIET_SUDO(cmd);
 */
#define EXEC_QUIET_SUDO(cmd)   CmdExec::run_quiet(cmd, true)

/** 
 * @brief runs ``` cmd ``` command, if commands fails throws std::runtime_error
 * @param cmd as scf::str1024 
 * @returns res.output, res.success, res.exit_code
 * @note 
 * auto res = EXEC_OR_THROW(cmd);
 */
#define EXEC_OR_THROW(cmd)     CmdExec::run_or_throw(cmd)

/** 
 * @brief runs ``` cmd ``` command with spinner
 * @param cmd as scf::str1024 
 * @returns res.output, res.success, res.exit_code
 * @note 
 * auto res = EXEC_SPINNER(cmd);
 */
#define EXEC_SPINNER(cmd)      CmdExec::run_spinner(cmd)

/** 
 * @brief runs ``` cmd ``` command as Sudo with spinner
 * @param cmd as scf::str1024 
 * @returns res.output, res.success, res.exit_code
 * @note 
 * auto res = EXEC_SPINNER(cmd);
 */
#define EXEC_SUDO_SPINNER(cmd) CmdExec::run_sudo_spinner(cmd)


