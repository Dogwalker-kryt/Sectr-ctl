/* 
 * Sectr-ctl (old. DriveMgr) - Linux Drive Management Utility
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

// ! Warning this version is the experimental version of the program,
// This version has the latest and newest functions, but may contain bugs and errors
// Current version of this code is in the VERSION macro below and in the line bellow
// v0.9.62.74_dev

// C++ libraries
#include <regex>
#include <cstdint>

// linux includes
#include <fcntl.h>        
#include <sys/ioctl.h>      
#include <sys/stat.h> 
#include <sys/statvfs.h>      
#include <linux/fs.h> 
#include <sys/mount.h>      
#include <cerrno>      

// openssl includes
#include <openssl/sha.h>

// custom includes
#include "../include/DmgrLib.h"
#include "../include/LDM_updater.h"
#include "../include/tests.hpp"
#include "../include/ui/MenuIO.hpp"
#include "../include/ui/Spinner.hpp"
#include "../include/ui/ListDrivesUtil.hpp"
#include "../include/ui/TerminalSize.hpp"
#include "../include/DiskMod.hpp"
#include "../include/cmd_exec/exec_cmd.h"

// ==== definitions ====
static scf::str16 VERSION("v0.9.63.76_dev");
static std::string version_str = VERSION.to_std_str();

// ========== Partition Management ========== 
// should be removed and replaced
class PartitionsUtils {
    private:
        // 1
        static bool resizePartition(const scf::str512& device, uint64_t newSizeMB) {
            try {

                const scf::str1024 cmd = "parted --script " + device + " resizepart 1 " + scf::to_str8(newSizeMB) + "MB";
                                 
                const auto res = EXEC(cmd);
                return res.success;

            } catch (const std::exception&) {

                ERR(ErrorCode::ProcessFailure, "Failed to resize partition");
                LOG_ERROR("Failed to resize partition");
                return false;

            }
        }

        // 2
        static bool movePartition(const scf::str512& device, int partNum, uint64_t startSectorMB) {
            try {

                const scf::str1024 cmd = "parted --script " + device + " move " + scf::to_str16(partNum) + " " + scf::to_str16(startSectorMB) + "MB";
                                 
                const auto res = EXEC_SUDO(cmd);
                return res.success;

            } catch (const std::exception&) {

                ERR(ErrorCode::ProcessFailure, "Failed to move partition");
                LOG_ERROR("Failed to move partition");
                return false;

            }
        }

        // 3
        static bool changePartitionType(const scf::str512& device, int partNum, const scf::str8 &newType) {
            try {

                const scf::str1024 backupCmd = "sfdisk -d " + device + " > " + device + "_backup.sf";
                EXEC_QUIET(backupCmd);

                const scf::str1024 cmd = "echo 'type=" + newType + "' | sfdisk --part-type " + device + " " + scf::to_str8(partNum);
                                 
                const auto res = EXEC(cmd); 
                const scf::str4096 output = res.output;
                return output.find("error") == scf::npos;

            } catch (const std::exception&) {

                ERR(ErrorCode::ProcessFailure, "Failed to change partition type");
                LOG_ERROR("Failed to change partition type");
                return false;

            }
        }

    public:
        static void case1ResizePartition(const std::vector<scf::str512> &partitions) {
            std::cout << "Enter partition number (1-" << partitions.size() << "): ";
            int partNum;
            std::cin >> partNum;

            if (!std::cin) {

                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                ERR(ErrorCode::InvalidInput, "Expected input is a integer");
                LOG_ERROR("Expected input is a integer");
                return;

            }

            if (partNum < 1 || partNum > (int)partitions.size()) {

                ERR(ErrorCode::OutOfRange, "Invalid partition number selected");
                return;

            }

            std::cout << "Enter new size in MB: ";
            uint64_t newSize;
            std::cin >> newSize;

            if (!std::cin) {

                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                ERR(ErrorCode::InvalidInput, "Expected a positive numeric input to assign to a uint64_t integer");
                LOG_ERROR("You cannot assign a number <=0 to a uint64 interger");
                return;

            }

            if (newSize == 0) {

                ERR(ErrorCode::OutOfRange, "newSize cannot be equal to 0; Expected a number greater then 0 for uint64_t integer");
                LOG_ERROR("newSize cannot be equal to 0");
                return;

            }

            askForConfirmation("[Warning] Resizing partitions can lead to data loss.\nAre you sure? ");

            if (PartitionsUtils::resizePartition(partitions[partNum-1], newSize)) {

                std::cout << "Partition resized successfully!\n";

            } else {

                ERR(ErrorCode::ProcessFailure, "Failed to resize partition");
                LOG_ERROR("Failed to resize partition");
                return;

            }
        }

        static void case2MovePartition(const std::vector<scf::str512> &partitions) {
            std::cout << "Enter partition number (1-" << partitions.size() << "): ";

            int partNum;
            std::cin >> partNum;

            if (!std::cin) {

                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                ERR(ErrorCode::InvalidInput, "Expected input is a integer");
                LOG_ERROR("Expected input is a integer");
                return;

            }

            if (partNum < 1 || partNum > (int)partitions.size()) {

                ERR(ErrorCode::OutOfRange, "Invalid partition number selected");
                return;

            }

            std::cout << "Enter new start position in MB: ";
            uint64_t startPos;
            std::cin >> startPos;

            if (!std::cin) {

                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                ERR(ErrorCode::InvalidInput, "Expected a positive numeric input to assign to a uint64_t integer");
                LOG_ERROR("You cannot assign a number <=0 to a uint64 interger");
                return;

            }

            if (startPos == 0) {

                ERR(ErrorCode::OutOfRange, "startPos cannot be equal to 0; Expected a number greater then 0 for uint64_t integer");
                LOG_ERROR("startPos cannot be equal to 0");
                return;

            }

            askForConfirmation("[Warning] Moving partitions can lead to data loss.\nAre you sure? ");

            if (movePartition(partitions[partNum-1], partNum, startPos)) {

                std::cout << "Partition moved successfully!\n";

            } else {

                ERR(ErrorCode::ProcessFailure, "Failed to move partition");
                LOG_ERROR("Failed to move partition");
                return;

            }
        }

        static void case3ChangePartitionType(const std::vector<scf::str512> &partitions, const scf::str512 &drive_name) {
            std::cout << "Enter partition number (1-" << partitions.size() << "): ";

            int partNum = scf::read<int>();

            if (partNum < 1 || partNum > (int)partitions.size()) {

                ERR(ErrorCode::OutOfRange, "Invalid partition number selected");
                return;

            }

            std::cout << "┌───────────────────────────┐\n";
            std::cout << "│ Available partition types │\n";
            std::cout << "├───────────────────────────┤\n";
            std::cout << "│ 1. Linux (83)             │\n";
            std::cout << "│ 2. NTFS (7)               │\n";
            std::cout << "│ 3. FAT32 (b)              │\n";
            std::cout << "│ 4. Linux swap (82)        │\n";
            std::cout << "└───────────────────────────┘\n";
            std::cout << "Enter type number: ";

            auto typeNum = InputValidation::getInt(1, 4);
            if (!typeNum.has_value()) return;

            scf::str8 newType;

            switch (*typeNum) {
                case 1: newType = "83"; break;
                case 2: newType = "7"; break;
                case 3: newType = "b"; break;
                case 4: newType = "82"; break;
                default:
                    ERR(ErrorCode::OutOfRange, "Invalid partition type selected");
                    break;
            }

            if (!newType.empty()) {

                askForConfirmation("[Warning] Changing partition type can make data inaccessible.\nAre you sure? ");

                if (changePartitionType(drive_name, partNum, newType)) {

                    std::cout << "Partition type changed successfully!\n";

                } else {

                    ERR(ErrorCode::ProcessFailure, "Failed to change partition type");
                    LOG_ERROR("Failed to change partition type");
                    return;

                }
            }
        }
};

static void listpartisions() { 
    const scf::str256 drive_name = ListDrivesUtil::listDrives(true); 

    scf::lnprintln("\nPartitions of drive ", drive_name, ":");

    const scf::str1024 cmd = "lsblk -o NAME,SIZE,TYPE,MOUNTPOINT,FSTYPE -n -p " + drive_name; 
    const auto res = EXEC_QUIET(cmd); 

    if (!res.success) {

        ERR(ErrorCode::ProcessFailure, "lsblk failed");
        LOG_ERROR("lsblk failed");

    }

    std::istringstream iss(res.output);
    std::string line;

    std::cout << std::left 
              << std::setw(2)
              << std::setw(4) << "/"
              << std::setw(10) << "Name" 
              << std::setw(10) << "Size" 
              << std::setw(10) << "Type" 
              << std::setw(15) << "Mountpoint" 
              << std::setw(10) << "FSType" 
              << "\n";
   scf::println(std::string(63, '-'));

    std::vector<scf::str512> partitions;

    while (std::getline(iss, line)) {

        if (line.find("part") != std::string::npos) {

            std::istringstream lss(line);
            std::string part_name, part_size, part_type, part_mount, part_fstype;
            
            lss >> part_name >> part_size >> part_type;
            
            // Get rest of line for mountpoint and fstype
            std::string rest;
            std::getline(lss, rest);
            std::istringstream rss(rest);
            rss >> part_mount >> part_fstype;
            
            if (part_mount == "-") part_mount = "";
            if (part_fstype == "-") part_fstype = "";
            
            // Print formatted row
            std::cout << std::left
                      << std::setw(18) << part_name
                      << std::setw(10) << part_size
                      << std::setw(10) << part_type
                      << std::setw(15) << part_mount
                      << std::setw(10) << part_fstype
                      << "\n";
            
            partitions.push_back(part_name);
        }
    }

    if (partitions.empty()) {
        ERR(ErrorCode::DeviceNotFound, "No partitions found on this drive");
    }

    scf::println("");

    const int choice = GenericMenuIO::noColorTuiMenu("Partition Management", {
        {1, "Resize partition"},
        {2, "Move partition"},
        {3, "Change partition type"},
        {0, "Return to main menu"}
    });

    switch (choice) {
        case 1: {
            PartitionsUtils::case1ResizePartition(partitions);
            break;
        }

        case 2: {
            PartitionsUtils::case2MovePartition(partitions);
            break;
        }

        case 3: {
            PartitionsUtils::case3ChangePartitionType(partitions, drive_name);
            break;
        }

        case 4:
            return;
            
        default:
            ERR(ErrorCode::OutOfRange, "Invalid option selected in partition menu");
    }
}


// ========== Disk Space Analysis ==========··−·

static void analyzeDiskSpace() {
    printFunctionHeader("Disk space analyze");
    const scf::str256 drive_name = ListDrivesUtil::listDrives(true); 

    scf::lnprintln((Globals::g_no_color ? BOLD : std::string(Globals::g_THEME_COLOR)), "┌────── Disk Information ──────", RESET);

    const scf::str1024 disk_cmd = "lsblk -b -o NAME,SIZE,TYPE,MOUNTPOINT -n -p " + drive_name;
    const auto disk_cmd_res = EXEC_QUIET(disk_cmd); 

    if (!disk_cmd_res.success || disk_cmd_res.output.empty()) {

        ERR(ErrorCode::ProcessFailure, "lsblk failed");
        return;

    }

    {
        std::istringstream iss(disk_cmd_res.output);
        std::string line;

        while (std::getline(iss, line)) {
            scf::println(Globals::g_THEME_COLOR, "│ ", RESET, line);
        }
        scf::println(Globals::g_THEME_COLOR, "│", RESET);
    }

    std::istringstream iss(disk_cmd_res.output);
    std::string line;
    bool found = false;
    std::string mount_point;
    std::string size;

    while (std::getline(iss, line)) {
        std::istringstream lss(line);

        std::string name, type;

        lss >> name >> size >> type;

        std::getline(lss, mount_point);

        if (!mount_point.empty() && mount_point[0] == ' ') mount_point = mount_point.substr(1);

        if (type == "disk") {
            found = true;
            std::cout << Globals::g_THEME_COLOR << "│ " << RESET << "Device:      " << name << "\n";
            try {

                unsigned long long bytes = std::stoull(size);
                const char* units[] = {"B", "KB", "MB", "GB", "TB"};
                int unit = 0;
                double human_size = bytes;

                while (human_size >= 1024 && unit < 4) {
                    human_size /= 1024;
                    ++unit;
                }

                std::cout << Globals::g_THEME_COLOR << "│ " << RESET << "Size:        " << human_size << " " << units[unit] << "\n";

            } catch (...) {
                std::cout << Globals::g_THEME_COLOR << "│ " << RESET << "Size:        " << size << " bytes\n";
            }

            std::cout << Globals::g_THEME_COLOR << "│ " << RESET << "Type:        " << type << "\n";
            std::cout << Globals::g_THEME_COLOR << "│ " << RESET << "Mountpoint:  " << (mount_point.empty() ? "-" : mount_point) << "\n";
        }
    }

    if (!found) {

        ERR(ErrorCode::DeviceNotFound, "No Disk found");
        return;

    } 

    if (!mount_point.empty() && mount_point != "-") {

        std::string df_cmd = "df -h '" + mount_point + "' | tail -1";
        const auto df_res = EXEC_QUIET(df_cmd); 
        std::string df_out = df_res.output;

        std::istringstream dfiss(df_out);

        std::string filesystem, df_size, used, avail, usep, mnt;
        dfiss >> filesystem >> df_size >> used >> avail >> usep >> mnt;

        scf::println(Globals::g_THEME_COLOR, "│", RESET);
        scf::println(Globals::g_THEME_COLOR, "│ ", RESET, "Used:        ", used);
        scf::println(Globals::g_THEME_COLOR, "│ ", RESET, "Available:   ", avail);
        scf::println(Globals::g_THEME_COLOR, "│ ", RESET, "Used %:      ", usep);

    } else {

        scf::println(Globals::g_THEME_COLOR, "│ ", RESET, "No mountpoint, cannot show used/free space.");

    }
    
    scf::println((Globals::g_no_color ? BOLD : std::string(Globals::g_THEME_COLOR)), "└──────────────────────────────", RESET);
}


// ========== Drive Formatting ==========
// should also be removed and replaced
class FormatUtils {
private:
    static bool confirm_format(const scf::str1024& drive, const scf::str32& label = "", const scf::str16& fs_type = "") {
        std::ostringstream msg;

        msg << "Are you sure you want to format: " << drive;

        if (!label.empty()) msg << " with label: " << label;

        if (!fs_type.empty()) msg << " and filesystem: " << fs_type;

        msg << "? (y/N)\n";
        
        scf::println(msg.str());

        auto confirmation = InputValidation::getChar({'y', 'n'});
        if (!confirmation.has_value()) return false;
        
        if (confirmation != 'y') {

            scf::println(RED, "[INFO] ", RESET, "Formatting cancelled by user");
            LOG_INFO("Formatting cancelled for drive: " + drive);
            return false;

        }

        return true;
    }

public:
    static void format_drive(const scf::str1024& drive_to_format, const scf::str32& label = "", const scf::str16& fs_type = "ext4") {
        if (!label.empty()) {

            if (label.length() > 16) {

                ERR(ErrorCode::OutOfRange, "Label too long (max 16 chars)");
                return;

            }

        }
        
        if (!confirm_format(drive_to_format, label, fs_type)) return;
        
        try {

            std::ostringstream cmd;

            cmd << "mkfs." << (fs_type.empty() ? "ext4" : fs_type);

            if (!label.empty()) cmd << " -L " << label;

            cmd << " " << drive_to_format;
            
            auto res = EXEC(scf::to_str1024(cmd.str()));
            
            if (!res.success) {

                ERR(ErrorCode::ProcessFailure, "Failed to format drive: " + drive_to_format);
                LOG_ERROR("Format failed: " + drive_to_format);
                return;

            }
            
            scf::println(res.output);
            scf::println(GREEN, "[INFO] Drive formatted successfully", RESET);

            LOG_INFO("Drive formatted: " + drive_to_format);
            
        } catch(const std::exception& e) {

            ERR(ErrorCode::ProcessFailure, "Exception during formatting: " + scf::str256(e.what()));
            LOG_ERROR("Format exception: " + scf::str256(e.what()));
            return;

        }
    }
    
    // Wrapper functions for backward compatibility
    static void formatDriveBasic(const scf::str512& drive) { format_drive(drive); }
    static void formatDriveWithLabel(const scf::str512& drive, const scf::str32& label) { format_drive(drive, label); }
    static void formatDriveWithLabelAndFS(const scf::str512& drive, const scf::str32& label, const scf::str16& fs) { format_drive(drive, label, fs); }
};

static void formatDrive() {
    scf::println("INFO: Standard formatting will automaticlly use ext4 filesystem");
    int fdinput = GenericMenuIO::noColorTuiMenu("Format", {{1, "Format drive"}, {2, "Format drive with label"}, {3, "Format drive with label and filesystem"}, {0, "Exit"} });
    
    switch (fdinput) {
        case 1:
            {
                scf::println("Choose a Drive to Format");
                const scf::str512 driveName = ListDrivesUtil::listDrives(true);

                FormatUtils::formatDriveBasic(scf::to_str512(driveName));
            }

            break;

        case 2:
            {
                scf::println("Choose a Drive to Format with label");
                const scf::str512 driveName = ListDrivesUtil::listDrives(true);

                scf::println("Enter label: ");
                auto label = InputValidation::getString();
                if (!label.has_value()) return;

                FormatUtils::formatDriveWithLabel(scf::to_str512(driveName), *label);
            }

            break;

        case 3:
            {
                scf::println("Choose a Drive to Format with label and filesystem type");
                const scf::str512 driveName = ListDrivesUtil::listDrives(true);

                scf::println("Enter label: ");
                auto label = InputValidation::getString();
                if (!label.has_value()) return;
                
                scf::println("Enter filesystem type (e.g. ext4, ntfs, vfat): ");
                auto fsType = InputValidation::getString();

                FormatUtils::formatDriveWithLabelAndFS(scf::to_str512(driveName), *label, *fsType);
            }

            break;

        case 0:
            break;

        default: {

            ERR(ErrorCode::OutOfRange, "Invalid formatting option selected");
            return;

        }
    }
}


// ========== Drive Health Check ==========

static void checkDriveHealth() {
    printFunctionHeader("Disk health");
    const scf::str512 driveHealth_name = ListDrivesUtil::listDrives(true);
    const scf::str1024 health_cmd = "smartctl -H " + driveHealth_name;
    const auto res = EXEC_QUIET_SUDO(health_cmd);
    const std::string health_output = StrUtils::removeFirstLines(res.output, 3); 
    scf::println(health_output);
   return;
}


// ========== Drive Resizing ==========
// should be removed and replaced
static void resizeDrive() {
    scf::lnprintln_flush("[Resize Drive]");
    const scf::str256 driveName = ListDrivesUtil::listDrives(true);

    scf::println("Enter new size in GB for drive ", driveName, ":");

    const auto new_size = InputValidation::getUint();
    if (!new_size.has_value()) return;

    if (new_size.value() == 0) {
        ERR(ErrorCode::OutOfRange, "new_size cannot be equal to 0; Expected a number greater then 0 for uint integer");
        return;
    }

    scf::println("Resizing drive ", driveName, " to ", std::to_string(new_size.value_or(0)), " GB...");

    try {

        const scf::str1024 resize_cmd = "sudo parted --script " + driveName +  " resizepart 1 " + scf::to_str32(new_size.value_or(0)) + "GB";
        const auto res = EXEC(resize_cmd);

        scf::println(res.output);

        if (!res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to resize drive: " + driveName);
            LOG_ERROR("Failed to resize drive: " + driveName);

        } else {

            scf::println(GREEN, "Drive resized successfully\n", RESET);
            LOG_SUCCESS("Drive resized successfully: " + driveName);

        }

    } catch (const std::exception& e) {

        ERR(ErrorCode::ProcessFailure, "Exception during resize: " + scf::str256(e.what()));
        LOG_ERROR("Exception during resize: " + scf::str256(e.what()));

    }
}


// ========== Drive Encryption ========== 
// new USB only encryption/decryption impl

class USBEnDeCryptionUtils {
private:
    enum class Metadata {
        TYPE,
        VENDOR,
        TRAN
    };

    struct MetadataHash {
        size_t operator()(Metadata m) const noexcept {
            return static_cast<size_t>(m);
        }
    };

    static const scf::str256 *isValidDrive(const scf::str256 &drive_name) {
        scf::str1024 cmd = "lsblk -o TYPE,VENDOR,TRAN -P -p " + drive_name; 
        auto res = EXEC_QUIET(cmd);

        if (!res.success || res.output.empty()) {

            ERR(ErrorCode::ProcessFailure, "lsblk failed to succed");
            LOG_ERROR("lsblk failed to succed");
            return nullptr;

        }

        std::unordered_map<Metadata, std::string, MetadataHash> meta;

        auto extract = [&](const std::string& key) -> std::string {
            std::string search = key + "=\"";
            size_t start = res.output.find(search);
            if (start == std::string::npos) return "N/A";

            start += search.length();
            size_t end = res.output.find("\"", start);
            if (end == std::string::npos) return "N/A";

            return res.output.substr(start, end - start);
        };

        meta[Metadata::TYPE] = extract("TYPE");
        meta[Metadata::VENDOR] = extract("VENDOR");
        std::string tran = StrUtils::toLowerString(meta[Metadata::TRAN] = extract("TRAN"));

        if (meta[Metadata::TYPE] != "disk") {

            ERR(ErrorCode::InvalidDevice, "Drive is not a Disk " + drive_name);
            LOG_ERROR("Drive is not a disk " + drive_name);
            return nullptr;

        }

        if (meta[Metadata::VENDOR] == "N/A" || meta[Metadata::VENDOR] == "ATA") {

            ERR(ErrorCode::InvalidDevice, "Drive is an internal disk " + drive_name + "; Expected USB Drive");
            LOG_ERROR("Drive is an internal Disk " + drive_name);
            return nullptr;
                        
        }

        if (tran != "usb") {

            ERR(ErrorCode::InvalidDevice, "Drive is not an USB Device " + drive_name + "; Expected USB Drive");
            LOG_ERROR("Drive is an internal Disk " + drive_name);
            return nullptr;
                        
        }

        const scf::str256 *val_disk = &drive_name;

        return val_disk;
    }

    static bool confirmationKeyInput() {
        scf::lnprintln("To proceed with anything you need to retype the following confirmation key:");

        const scf::str<10> confirmation_key = confirmationKeyGenerator();
        scf::lnprintln(confirmation_key);

        scf::lnprintln("retype the key:");

        scf::str<10> user_retyped_key = scf::read<scf::str<10>>();

        if (!Globals::bypass_security_code && user_retyped_key != confirmation_key) {

            scf::println(YELLOW, "[INFO] ", RESET, "The key you retyped doesnt match the original key\n", "Process Aborted due to invalid input");
            LOG_INFO("The retyped key doesnt match the original key; Process Aborted due to invalid input");

            scf::println("Do you want to retry? (y/N)");

            const auto confirm_if_retry = InputValidation::getChar({'y', 'n'});
            if (!confirm_if_retry.has_value()) return false;

            if (confirm_if_retry == 'n') {

                scf::println(YELLOW, "[INFO] ", RESET, "User aborted retry");
                LOG_INFO("Key retry was aborted by the user");
                return false;

            }

            scf::str<10> confirm_key2 = confirmationKeyGenerator();

            scf::lnprintln("[last chance] Retype the following confirmation key:");
            scf::lnprintln(confirm_key2);

            scf::str<10> confirm_key2_input = scf::read<scf::str<10>>();

            if (!Globals::bypass_security_code && confirm_key2_input != confirm_key2) {

                scf::println(YELLOW, "[INFO] ", RESET, "The key you retyped doesnt match the original key\n", "Process Aborted due to invalid input");
                LOG_INFO("The retyped key doesnt match the original key; Process Aborted due to invalid input");
                return false;

            }

            return true;
        }

        return true;
    }

    static void encryptUSBDrive(const scf::str256 &drive_name) {
        //passphrases
        scf::lnprintln(BOLD, "[Encryption of ", drive_name, "]", RESET);
        scf::str64 passphrase, passphrase_retype;
        
        scf::lnprintln(RED, "[WARNING] ", RESET, "You should save or remember the passphrase!\n The Sectrctl will NOT! save it");
        scf::lnprintln("Enter a Passphrase for the encrypted USB");
        scf::read(passphrase);

        if (passphrase.empty()) {

            ERR(ErrorCode::InvalidInput, "The passphrase you entered is emtpy; Expecting non empty string");
            LOG_ERROR("The passphrase you entered is emtpy");
            return;

        }

        scf::lnprintln("Retype your Passphrase you just entered:");
        scf::read(passphrase_retype);
    
        if (passphrase.empty() || passphrase_retype.empty()) {

            ERR(ErrorCode::InvalidInput, "The passphrase you entered is emtpy; Expecting non empty string");
            LOG_ERROR("The passphrase you entered is emtpy");
            return;

        }

        if (passphrase != passphrase_retype) {

            ERR(ErrorCode::InvalidInput, "The passphrases you entered doesnt match; Expecting similar passphrase input");
            LOG_ERROR("The passphrases you entered doesnt match: p2:'" + passphrase_retype + "'");
            return;

        }
    
        std::ofstream tmpfile("/tmp/LDM_tmp_dump.txt");
        tmpfile << passphrase_retype;
        tmpfile.close();

        // pass passphrases to crypsetup
        const scf::str1024 cryptsetup_cmd = "cryptsetup luksFormat " + drive_name + " --key-file=/tmp/LDM_tmp_dump.txt -q && shred /tmp/LDM_tmp_dump.txt";
        const auto cryptsetup_res = EXEC_SUDO(cryptsetup_cmd);
    
        if (!cryptsetup_res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to execute cryptsetup on: " + drive_name);
            LOG_ERROR("Failed to execute cryptsetup on: " + drive_name);
            return;

        }

        // open encrypted device
        scf::println("[INFO] open encrypted device...");
        const scf::str8 mapper_name = "enc_usb";
        const scf::str32 mapper_path = "/dev/mapper/" + mapper_name;

        const scf::str1024 cryptsetup_open_cmd = "echo \"" + passphrase + "\" | cryptsetup open " + drive_name + " " + mapper_name + " --key-file=-";
        const auto cryptsetup_open_res = EXEC_SUDO(cryptsetup_open_cmd);

        if (!cryptsetup_open_res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to open drive crypsetup on: " + drive_name);
            LOG_ERROR("Failed to open drive with cryptsetup on: " + drive_name);
            return;

        }       

        // When you need a diffrent FS then change it here
        const std::string mkfs_ext4_cmd = "mkfs.ext4 " + mapper_path;
        const auto mkfs_res = EXEC_SUDO(mkfs_ext4_cmd);

        if (!mkfs_res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to make FS on " + mapper_name);
            LOG_ERROR("Failed to make FS on " + mapper_name);
            return;            

        }

        // mount encrypted device
        scf::println("[INFO] mounting encrypted device...");
        const std::string mount_cmd = "mount " + mapper_path + " /media/" + mapper_name;
        const auto mk_mountpoint_res = EXEC_SUDO("mkdir -p /media/" + mapper_name);
        const auto mount_res = EXEC_SUDO(mount_cmd);

        if (!mk_mountpoint_res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to create mountpoint");
            LOG_ERROR("Failed to create mountpoint");
            return;     

        }

        if (!mount_res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to mount " + mapper_name);
            LOG_ERROR("Failed to mount " + mapper_name);
            return;            

        }    

        // close 
        scf::println("[INFO] closing encrypted device...");
        const auto unmount_cryptsetup_res = EXEC_SUDO("umount /media/" + mapper_name);

        if (!unmount_cryptsetup_res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to unmount /media/" + mapper_name);
            LOG_ERROR("Failed to unmount /media/" + mapper_name);
            return;

        }

        const std::string close_cryptsetup_cmd = "cryptsetup close " + mapper_name;
        const auto close_cryptsetup_res = EXEC_SUDO(close_cryptsetup_cmd);

        if (!close_cryptsetup_res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to close " + mapper_name);
            LOG_ERROR("Failed to close " + mapper_name);
            return;            

        }

        std::fill(passphrase.begin(), passphrase.end(), '\0');
        std::fill(passphrase_retype.begin(), passphrase_retype.end(), '\0');

        scf::lnprintln(GREEN, "[SUCCESS] ", RESET, "Encryption completed successfully");
        LOG_SUCCESS("Encryption completed successfully");
        // TODO: maby custom listdrives func for printing only usb's; make that passphrse dont leak into shell
    }

    static void decryptUSBDrive(const scf::str256 &drive_name) {
        scf::lnprintln(BOLD, "[Decryption / Unlock of ", drive_name, "]", RESET);

        scf::str64 passphrase;

        scf::lnprintln(RED, "[WARNING] ", RESET, "You must enter the correct passphrase to unlock this encrypted USB.");

        scf::lnprintln("Enter the Passphrase:");
        scf::read(passphrase);

        if (passphrase.empty()) {

            ERR(ErrorCode::InvalidInput, "Passphrase empty; expected non-empty string");
            LOG_ERROR("Passphrase empty");
            return;

        }

        // mapper name
        const scf::str8 mapper_name = "enc_usb";
        const scf::str32 mapper_path = "/dev/mapper/" + mapper_name;

        // cryptsetup open
        const scf::str1024 cryptsetup_open_cmd = "echo \"" + passphrase + "\" | cryptsetup open " + drive_name + " " + mapper_name + " --key-file=-";

        const auto cryptsetup_open_res = EXEC_SUDO(cryptsetup_open_cmd);

        if (!cryptsetup_open_res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to open encrypted device with cryptsetup");
            LOG_ERROR("Failed to open encrypted device with cryptsetup");
            return;

        }

        // mount
        const auto mk_mountpoint_res = EXEC_SUDO("mkdir -p /media/" + mapper_name);

        if (!mk_mountpoint_res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to create mountpoint");
            LOG_ERROR("Failed to create mountpoint");
            return;

        }

        const scf::str1024 mount_cmd = "mount " + mapper_path + " /media/" + mapper_name;
        const auto mount_res = EXEC_SUDO(mount_cmd);

        if (!mount_res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to mount decrypted device");
            LOG_ERROR("Failed to mount decrypted device");
            return;

        }

        scf::lnprintln(GREEN, "[SUCCESS] ", RESET , "USB successfully unlocked and mounted at /media/", mapper_name);
        LOG_SUCCESS("USB successfully unlocked and mounted");

        scf::println(YELLOW, "[INFO] ", RESET, "Press ENTER when you are done using the USB to unmount and lock it again.");
        std::cin.get();

        // unmount
        const auto unmount_res = EXEC_SUDO("umount /media/" + mapper_name);

        if (!unmount_res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to unmount decrypted device");
            LOG_ERROR("Failed to unmount decrypted device");
            return;

        }

        // close
        const scf::str1024 close_cmd = "cryptsetup close " + mapper_name;
        const auto close_res = EXEC_SUDO(close_cmd);

        if (!close_res.success) {

            ERR(ErrorCode::ProcessFailure, "Failed to close mapper device");
            LOG_ERROR("Failed to close mapper device");
            return;

        }

        // wipe passphrase from memory
        std::fill(passphrase.begin(), passphrase.end(), '\0');

        scf::lnprintln(GREEN, "[SUCCESS] ", RESET, "USB successfully locked and unmounted.");
        LOG_SUCCESS("USB successfully locked and unmounted");
    }

    static void cryptionAltMenu(const std::string &drive_name) {
        scf::lnprintln("Do you want to Encrypt or Decrypt your USB? (e/d):");

        const auto e_or_d = InputValidation::getChar({'e', 'd'});
        if (!e_or_d.has_value()) return;

        if (e_or_d == 'e') {

            encryptUSBDrive(drive_name);
            return;

        } else if (e_or_d == 'd') {

            decryptUSBDrive(drive_name);
            return;

        }
    }

public:
    static void mainUsbEnDecryption() {
        printFunctionHeader("USB De/Encryption");

        const scf::str256 drive_name = ListDrivesUtil::listDrives(true);

        {
            const scf::str256 *val_drive_name = isValidDrive(drive_name);
            
            if (val_drive_name == nullptr) {

                ERR(ErrorCode::InvalidDevice, "'" + drive_name + "' Couldnt get validated; Expected USB Drive");
                LOG_ERROR("'" + drive_name + "' Couldnt get validated");
                return;

            }
        }

        scf::lnprintln(YELLOW, "[Warning] ", RESET, "Are you sure you want to en- or decrypt: '", drive_name, "' ? (y/N)");
    
        const auto confirmation = InputValidation::getChar({'y', 'n'});
        if (!confirmation.has_value()) return;

        const bool is_confirm_key_true = confirmationKeyInput();

        if (!is_confirm_key_true) {

            LOG_INFO("ConfirmKeyInput is false, aborting operation");
            return;

        }

        if (confirmation == 'y') {

            scf::lnprintln("Proceeding...");
            cryptionAltMenu(drive_name);

        } else if (confirmation == 'n') {

            scf::println(YELLOW, "[INFO] ", RESET, "En- Decryption with '", drive_name, "' was aborted by the user");
            LOG_INFO("En- Decryption with '" + drive_name + "' was aborted by the user");
            return;

        }
    }
};


// ========== Drive Data Overwriting ==========
// Tried my best to make this as safe and readable and maintainable as possible. v0.9.12.92

static void overwriteDriveData() { 
    printFunctionHeader("Disk Overwriting");
    const scf::str256 drive_to_operate_on = ListDrivesUtil::listDrives(true);

    scf::println(YELLOW, "[WARNING]", RESET, " Are you sure you want to overwrite all data on ", BOLD, drive_to_operate_on, RESET, "? This action cannot be undone! (y/n)");
        
    const auto confirm = InputValidation::getChar({'y', 'n'});
    if (!confirm.has_value()) return;

    if (confirm != 'y') {

        scf::println(BOLD, "[Overwriting aborted]", RESET, " The Overwriting process of ", drive_to_operate_on, " was interupted by user");
        LOG_INFO("Overwriting process aborted by user for drive: " + drive_to_operate_on);
        return;

    }

    if (!Globals::bypass_security_code) {

        scf::lnprintln("To be sure you want to overwrite the data on ", BOLD, drive_to_operate_on, RESET, " you need to enter the following safety key");

        scf::str<10> conf_key = confirmationKeyGenerator();
        LOG_INFO("Confirmation key generated for overwriting drive: " + drive_to_operate_on);

        scf::println(conf_key);
        scf::lnprintln("Enter the confirmation key:");

        const auto user_input = InputValidation::getString(10);
        if (!user_input.has_value()) return;

        if (user_input.value() != conf_key.to_std_str()) {

            scf::println(BOLD, "[INFO]", RESET, " The confirmationkey was incorrect, the overwriting process has been interupted\n");
            LOG_INFO("Incorrect confirmation key entered, overwriting process aborted for drive: " + drive_to_operate_on);
            return;

        }
    }

    scf::lnprintln(YELLOW, "[Process]", RESET, " Proceeding with overwriting all data on: ", drive_to_operate_on);
    scf::println(" \n");

    // const auto res_urandom = EXEC_SUDO_SPINNER("dd if=/dev/urandom of=" + drive_to_operate_on + " bs=16M >/dev/null 2>&1 && sync"); 
    const auto res_zero = EXEC_SUDO_SPINNER("dd if=/dev/zero of=" + drive_to_operate_on + " bs=16M >/dev/null 2>&1 && sync"); 
            
    // if (!res_urandom.success && !res_zero.success) {

    //     ERR(ErrorCode::ProcessFailure, "Failed to overwrite the drive: " + drive_to_operate_on);
    //     LOG_ERROR("Overwriting failed to complete for drive: " + drive_to_operate_on);
    //     return;

    // } else if (!res_urandom.success || !res_zero.success) {

    //     scf::println(YELLOW, "[Warning]", RESET, " One of the overwriting operations failed, but the drive may have been partially overwritten. Please check the output and try again if necessary.");
    //     LOG_WARNING("One of the overwriting operations failed for drive: " + drive_to_operate_on);
    //     return;
    // } 

    if (!res_zero.success) {
        ERR(ErrorCode::ProcessFailure, "Failed to overwrite the drive: " + drive_to_operate_on);
        LOG_ERROR("Overwriting failed to complete for drive: " + drive_to_operate_on);
        return;
    }

    scf::println(GREEN, "[Success]", RESET, " Overwriting completed successfully for drive: ", drive_to_operate_on);
    LOG_SUCCESS("Overwriting completed successfully for drive: " + drive_to_operate_on);
    return;
}

class OverwriteUtility {
private:
    static bool confirm_key(const scf::str512& drive_to_op) {
        if (Globals::bypass_security_code) {
            return true;
        }
        
        scf::lnprintln("To be sure you want to overwrite the data on ", BOLD, drive_to_op, RESET, " you need to enter the following safety key");

        scf::str<10> conf_key = confirmationKeyGenerator();
        if (conf_key.empty()) { 
            ERR(ErrorCode::DataUnavailable, "confirmation Key is empty");
            LOG_ERROR("confirmation key is emtpy");
            return false;
        }

        LOG_INFO("Confirmation key generated for overwriting drive: " + drive_to_op + " key: " + conf_key);

        scf::println(conf_key);
        scf::lnprintln("Enter the confirmation key:");

        const auto user_input = InputValidation::getString(10);
        if (!user_input.has_value()) return false;

        if (user_input.value() != conf_key.to_std_str()) {

            scf::println(BOLD, "[INFO]", RESET, " The confirmationkey was incorrect, the overwriting process has been interupted\n");
            LOG_INFO("Incorrect confirmation key entered, overwriting process aborted for drive: " + drive_to_op);
            return false;

        }        

        return true;
    }

    static scf::result<int, ErrorCode> drive_fd(const scf::str512& drive_to_op) {
        const int fd = open(drive_to_op.c_str(), O_RDWR | O_EXCL | O_CLOEXEC);

        if (fd < 0) {
            ERR(ErrorCode::IOError, "Failed to open drive for overwriting: " + drive_to_op);
            LOG_ERROR("Failed to open drive for overwriting: " + drive_to_op);
            return scf::result<int, ErrorCode>::err(ErrorCode::IOError);
        }         

        return scf::result<int, ErrorCode>::ok(fd);
    }

    static const uint64_t read_sysfs_uint64(const scf::str512 &__restrict__ path) {
        std::ifstream sys_file(path.c_str());

        uint64_t value = 0;

        if (!(sys_file >> value)) {
            return 0;
        }

        return value;
    }

    static scf::str_t *mount_point_str(const scf::str512 &disk) {
        FILE *fp = fopen("/proc/self/mountinfo", "r");

        scf::str_t line;
        static scf::str_t res_mpoint;
        scf::str64 mount_point;
        scf::str32 device;

        while(fgets(line.data(), line.capacity(), fp)) {
            std::istringstream iss(line);

            if (!(iss >> device >> mount_point)) {
                continue;
            }

            if (device == disk) {
                res_mpoint = mount_point;
                fclose(fp);
                return &res_mpoint;
            }
        }

        fclose(fp);
        return nullptr;
    }

    typedef struct {
        bool blkdiscard = false;
        bool blksecdiscard = false;
    } support_t;
    
    static const support_t check_support(const scf::str512 &disk) {
        support_t support;
        const uint64_t discard_max = read_sysfs_uint64("/sys/block/" + disk.substr(5, disk.length()) + "/queue/discard_max_bytes");
        const uint64_t discard_granualarity = read_sysfs_uint64("/sys/block/" + disk.substr(5, disk.length()) + "/queue/discard_granualarity");
           
        support.blkdiscard = (discard_granualarity > 0 && discard_max > 0);
        support.blksecdiscard = support.blkdiscard && (read_sysfs_uint64("/sys/block/" + disk.substr(5, disk.length()) + "/queue/secure_discard") > 0);

        return support;
    } 

    static bool om_perform_secure_discard(int fd, uint64_t &size) {
        if (ioctl(fd, BLKSECDISCARD, &size) < 0) {
            ERR(ErrorCode::IOError, "Secure discard failed");
            LOG_ERROR("Secure discard failed");

            return false;
        }
        return true;
    }

    static bool om_perform_discard(int fd, uint64_t &size) {
        if (ioctl(fd, BLKDISCARD, &size) < 0) {
            ERR(ErrorCode::IOError, "Discard failed");
            LOG_ERROR("Discard failed");

            return false;
        }
        return true;
    }

    static bool om_perform_zeroout(int fd, uint64_t &size) {
        constexpr size_t buffer_size = 4096;
        char buffer[buffer_size] = {0};
        uint64_t remaining = size;
        size_t to_write{};

        while (remaining > 0) {
            to_write = std::min(buffer_size, static_cast<size_t>(remaining));

            if (write(fd, buffer, to_write) != static_cast<ssize_t>(to_write)) {
                ERR(ErrorCode::IOError, "Failed to write zeros to drive");
                LOG_ERROR("Failed to write zeros to drive");
    
                return false;
            }
            remaining -= to_write;
        }
        return true;
    }

public:
    static void overwriter() {
        printFunctionHeader("Disk Overwriting");
        const scf::str256 drive_to_op = ListDrivesUtil::listDrives(true);

        scf::println(YELLOW, "[WARNING]", RESET, " Are you sure you want to overwrite all data on ", BOLD, drive_to_op, RESET, "? This action cannot be undone! (y/n)");
        
        const auto confirm = InputValidation::getChar({'y', 'n'});
        if (!confirm.has_value()) return;

        if (confirm != 'y') {

            scf::println(BOLD, "[Overwriting aborted]", RESET, " The Overwriting process of ", drive_to_op, " was interupted by user");
            LOG_INFO("Overwriting process aborted by user for drive: " + drive_to_op);
            return;

        }

        bool bconfirm = confirm_key(drive_to_op);
        if (!bconfirm) return;

        scf::lnprintln(CYAN, "[Process]", RESET, " Proceeding with overwriting all data on: ", drive_to_op);
        scf::println(" \n");

        const scf::str_t *disk_mount_point = mount_point_str(drive_to_op);
        if (disk_mount_point != nullptr) {
            scf::println(YELLOW, "[WARNING] ", RESET, " The drive ", BOLD, "'", drive_to_op, "'", RESET, " is currently mounted at ", BOLD, *disk_mount_point, RESET, 0x00, "Please umount the drive!");
            LOG_WARNING(drive_to_op + " is currently mounted at " + *disk_mount_point);
            return;
        } 

        scf::result<int, ErrorCode> fd = drive_fd(drive_to_op);
        if (fd.has_error()) return;

        struct stat st{};
        if (fstat(fd.value(), &st) < 0 || !S_ISBLK(st.st_mode)) {
            ERR(ErrorCode::IOError, "Failed to get drive size for overwriting: " + drive_to_op);
            LOG_ERROR("Failed to get drive size for overwriting: " + drive_to_op);
            close(fd.value());
            return;
        }

        uint64_t drive_size = 0;
        if (ioctl(fd.value(), BLKGETSIZE64, &drive_size) < 0) {
            ERR(ErrorCode::IOError, "Failed to get drive size for overwriting: " + drive_to_op);
            LOG_ERROR("Failed to get drive size for overwriting: " + drive_to_op);
            close(fd.value());
            return;
        }

        static support_t support = check_support(drive_to_op);

        bool write_success = false;
        if (support.blksecdiscard) {
            scf::println(CYAN, "[DEBUG] ", RESET, "using secure discard (BLKSECDISCARD) for ", drive_to_op);
            write_success = om_perform_secure_discard(fd.value(), drive_size);
        } else if (support.blkdiscard) {
            scf::println(CYAN, "[DEBUG] ", RESET, "using discard (BLKDISCARD) for ", drive_to_op);
            write_success = om_perform_discard(fd.value(), drive_size);
        } else {
            scf::println(YELLOW, "[DEBUG] ", RESET, "using fallback zeroing out for ", drive_to_op);
            write_success = om_perform_zeroout(fd.value(), drive_size);
        }

        close(fd.value());

        if (write_success) {
            scf::println(GREEN, "[SUCCESS]", RESET, " Drive ", drive_to_op, " has been successfully overwritten.");
            LOG_SUCCESS("Drive " + drive_to_op + " has been successfully overwritten.");
            return;
        } 

        scf::println(RED, "[FAILURE]", RESET, " Failed to overwrite drive ", drive_to_op);
        ERR(ErrorCode::IOError, "Failed to overwrite " + drive_to_op);
        LOG_ERROR("Failed to overwrite drive " + drive_to_op);

        return;
    }
};

// ========== Drive Metadata Reader ==========

class MetadataReader {
private:
    static DriveMetadata *getMetadata(DriveMetadata *metadata, const scf::str512& drive) {
        // -P (Pairs) is the key here. It output KEY="VALUE"
        const scf::str1024 cmd = "lsblk -o NAME,SIZE,MODEL,SERIAL,TYPE,MOUNTPOINT,VENDOR,FSTYPE,UUID -P -p " + drive; 

        const auto res = EXEC_QUIET(cmd);

        if (!res.success || res.output.empty()) { 

            ERR(ErrorCode::ProcessFailure, "The lsblk failed to deliver data");
            LOG_ERROR("lsblk failed to deliver data");
            return nullptr; 
            
        }

        metadata->name       = extractt("NAME", res.output);
        metadata->size       = extractt("SIZE", res.output);
        metadata->model      = extractt("MODEL", res.output);
        metadata->serial     = extractt("SERIAL", res.output);
        metadata->type       = extractt("TYPE", res.output);
        metadata->mountpoint = extractt("MOUNTPOINT", res.output);
        metadata->vendor     = extractt("VENDOR", res.output);
        metadata->fstype     = extractt("FSTYPE", res.output);
        metadata->uuid       = extractt("UUID", res.output);

        return metadata;
    }

    static void displayMetadata(const DriveMetadata *metadata) {
        scf::println_flush("┌──────── Drive Metadata ─────────");

        auto printAttr = [&](const std::string& attr, const std::string& value) {
            scf::println("│ ", attr, ": ", (value.empty() ? "N/A" : value));
        };

        printAttr("Name", metadata->name.value_or("[ERROR] No Data available"));
        printAttr("Size", metadata->size.value_or("[ERROR] No Data available"));
        printAttr("Model", metadata->model.value_or("[ERROR] No Data available"));
        printAttr("Serial", metadata->serial.value_or("[ERROR] No Data available"));
        printAttr("Type", metadata->type.value_or("N/A"));
        printAttr("Mountpoint", metadata->mountpoint.value_or("Not mounted"));
        printAttr("Vendor", metadata->vendor.value_or("N/A"));
        printAttr("Filesystem", metadata->fstype.value_or("N/A"));
        printAttr("UUID", metadata->uuid.value_or("N/A"));

        if (!Globals::smart_data) return;

        if (*metadata->type == "disk") {

            scf::lnprintln("┌-─-─-─- SMART Data -─-─-─-─");
            
            const scf::str1024 smartCmd = "smartctl -i " + *metadata->name;
            const auto res = EXEC_QUIET_SUDO(smartCmd); 

            if (!res.success) {

                ERR(ErrorCode::ProcessFailure, "Failed to retrieve SMART data for " + *metadata->name);
                LOG_ERROR("Failed to retrieve SMART data for " + *metadata->name);
                return;

            }

            const scf::str1024 smartOutput = StrUtils::removeFirstLines(res.output, 4);

            if (!smartOutput.empty()) {

                scf::print(smartOutput);

            } else {

                ERR(ErrorCode::CorruptedData, "Failed to retrieve SMART data for " + *metadata->name);
                LOG_ERROR("Failed to retrieve SMART data for " + *metadata->name);

            }
        }   
        scf::println("└─  - -─ --- ─ - -─-  - ──- ──- ───────────────────");         
    } 
    
public:
    static void mainReader() {
        printFunctionHeader("Metadata viewer");
        DriveMetadata *metadata = new DriveMetadata;
        
        const scf::str256 driveName = ListDrivesUtil::listDrives(true);
        metadata = getMetadata(metadata, driveName);

        if (metadata == nullptr) {

            ERR(ErrorCode::ProcessFailure, "Failed to read metadata for drive: " + driveName);
            LOG_ERROR("Failed to read metadata for drive: " + driveName);
            return;
                
        }

        displayMetadata(metadata);
        LOG_SUCCESS("Successfully read metadata for drive: " + driveName);
        
        delete metadata;
    } 
};


// ========== Mounting and Burning Utilities ==========
// IsoFileMetadataChecker and IsoBurner refactored; v0.9.13.93

class MountUtility {
private:
    static const char *getIsoPath() {
        char iso_path[256];

        scf::flush_stdin();
        fgets(iso_path, 256, stdin);

        scf::str1024 validated_iso_path = filePathHandler(iso_path);

        return validated_iso_path.c_str();
    }

    static bool CopyISOtoUSB(const int usb_fd, const int iso_fd) {
        constexpr size_t BYTE_SIZE = 16 * 1024 * 1024;
    
        char byte_buffer[BYTE_SIZE];
        ssize_t bytes_read = 0;

        while (true) {

            do {
                bytes_read = read(iso_fd, byte_buffer, sizeof(byte_buffer));
            } while (bytes_read < 0 && errno == EINTR);

            if (bytes_read < 0) {
                perror("read ISO");
                return false;
            }

            // EOF
            if (bytes_read == 0) {
                break;
            }

            // Write the entire chunk.
            ssize_t total_written = 0;

            while (total_written < bytes_read) {
                ssize_t bytes_written = write(usb_fd, byte_buffer + total_written, bytes_read - total_written );

                if (bytes_written < 0) {
                    if (errno == EINTR) {
                        continue;
                    }

                    perror("write USB");
                    return false;
                }

                // Shouldn't normally happen, but prevents an infinite loop.
                if (bytes_written == 0) {
                    std::fprintf(stderr, "write returned 0\n");
                    return false;
                }

                total_written += bytes_written;
            }
        }

        if (fsync(usb_fd) < 0) {
            perror("fsync USB");
            return false;
        }

        return true; 
    }

    static void BurnISOToStorageDevice() {
        printFunctionHeader("Bootable USB creator");
        const scf::str256 drive_name = ListDrivesUtil::listDrives(true);

        scf::lnprintln("Enter path to iso. Format is ", BOLD , "'/path/file.iso'", RESET);
        const char *iso_path = getIsoPath();

        int iso_fd = open(iso_path, O_RDONLY);
        if (iso_fd < 0) {
            ERR(ErrorCode::IOError, "Failed to open ISO file: " + scf::str_t(iso_path));
            LOG_ERROR("Failed to open ISO file: " + scf::str_t(iso_path));
            return;
        }

        int usb_fd = open(drive_name.c_str(), O_WRONLY);
        if (usb_fd < 0) {
            ERR(ErrorCode::IOError, "Failed to open USB device: " + drive_name);
            LOG_ERROR("Failed to open USB device: " + drive_name);
            close(iso_fd);
            return;
        }

        if (!CopyISOtoUSB(usb_fd, iso_fd)) {
            ERR(ErrorCode::ProcessFailure, "Failed to copy ISO to USB device: " + drive_name);
            LOG_ERROR("Failed to copy ISO to USB device: " + drive_name);
        } else {
            scf::lnprintln(GREEN, "[SUCCESS] Successfully burned ISO to ", drive_name, RESET);
            LOG_SUCCESS("Successfully burned ISO to drive: " + drive_name);
        }
    }

    /**
     * @brief wrapps the orgirnal unmount() and mount() funcs to gether in one
     * @param mount_or_unmount type in mount, you will get the mount function, type in unmount you will get the unmount fukntion
     */
    static void choose_mount_unmount(const std::string &mount_or_unmount) {
        if (mount_or_unmount == "mount") {

            scf::lnprintln("[Mounting]");
            scf::println("Enter the drive you want to mount:");
            const scf::str256 drive_name = ListDrivesUtil::listDrives(true);

            scf::lnprintln("Enter the name for the drive under the name its mounted under '/mnt/':");

            scf::flush_stdin();

            scf::str512 mount_name;
            scf::read(mount_name);

            if (mount_name.size() > 64) {

                ERR(ErrorCode::InvalidInput, "Mount name too long; Expected max length of 64 characters");
                LOG_ERROR("Mount name too long: " + mount_name);
                return;

            }

            if (mount_name.empty()) {

                ERR(ErrorCode::InvalidInput, "Mount name cannot be empty");
                LOG_ERROR("Mount name cannot be empty");
                return;

            }

            const char* invalid_chars[7] = {"-", "'", "&", "<", "|", ">", ";"};

            for (size_t i = 0; i < 7; ++i) {
                if (size_t pos = mount_name.find(invalid_chars[i]); pos != scf::npos) {

                    ERR(ErrorCode::InvalidInput, "Invalid characters in mount name: " + mount_name);
                    LOG_ERROR("Invalid characters in mount name");
                    return;

                }
            }

            const scf::str512 mount_cmd = "mount " + drive_name + " /mnt/" + mount_name;
            const auto mount_res = EXEC_SUDO(mount_cmd);
             
            if (!mount_res.success) {

                ERR(ErrorCode::ProcessFailure, "Couldnt mount '" + drive_name + "' at '/mnt/" + mount_name);
                LOG_ERROR("Couldnt mount '" + drive_name + "' at '/mnt/" + mount_name);
                return;

            }

        } else if (mount_or_unmount == "unmount") {

            scf::lnprintln("[Unmounting]");
            const scf::str256 drive_to_unmount = ListDrivesUtil::listDrives(true);
            if (umount(drive_to_unmount.c_str()) < 0) {
                ERR(ErrorCode::IOError, "Failed to unmount drive: " + drive_to_unmount);
                LOG_ERROR("Failed to unmount drive: " + drive_to_unmount);
            }
        }

        return;
    }

    static void Restore_USB_Drive() {
        scf::lnprintln_flush("Choose the USB/Drive you want to restore (overwrite with empty filesystem and partition table):");
        const scf::str256 restore_device_name = ListDrivesUtil::listDrives(true);

        try {

            scf::lnprintln("\nAre you sure you want to overwrite/clean the ISO/Disk_Image from: ", BOLD, restore_device_name, RESET, " ? [y/n]");
            
            const auto restore_confirm = InputValidation::getChar({'y', 'n'});
            if (!restore_confirm.has_value()) return;

            if (restore_confirm != 'y') {

                scf::lnprintln(CYAN, "[INFO] ", RESET, "Operation cancelled");
                LOG_INFO("restore usb operation cancelled");
                return;

            }

            scf::lnprintln(CYAN, "[Phase 1]:", RESET);

            EXEC_SUDO_SPINNER("umount " + restore_device_name + "* 2>/dev/null || true");
            
            const auto wipefs_res = EXEC_QUIET_SUDO("wipefs -a " + restore_device_name + " >/dev/null 2>&1 && sync");

            if (!wipefs_res.success) {

                ERR(ErrorCode::ProcessFailure, "Failed to wipe device: " + restore_device_name);
                LOG_ERROR("Failed to wipe the filesystem of " + restore_device_name);
                return;

            }

            // Zero out start
            scf::println(CYAN, "[Phase 2]:", RESET);

            const auto dd_res = EXEC_SUDO_SPINNER("dd if=/dev/zero of=" + restore_device_name + " bs=1M count=10 >/dev/null 2>&1 && sync");

            if (!dd_res.success) {

                LOG_ERROR("Failed to overwrite the iso image on the usb");
                ERR(ErrorCode::ProcessFailure, "Failed to overwrite device: " + restore_device_name);
                return;

            }

            // Create partition table
            scf::println(CYAN, "[Phase 3]:", RESET);
            const auto parted_res = EXEC_SUDO_SPINNER("parted -s " + restore_device_name + " mklabel msdos mkpart primary 1MiB 100%");

            if (!parted_res.success) {

                LOG_ERROR("Failed while restoring USB: " + restore_device_name);
                ERR(ErrorCode::ProcessFailure, "Failed to create partition table on USB device: " + restore_device_name);
                return;

            }

            // Probe partitions
            scf::println(CYAN, "[Phase 4]:", RESET);
            const auto partprobe_res = EXEC_SUDO("partprobe " + restore_device_name);

            if (!partprobe_res.success) { 

                ERR(ErrorCode::ProcessFailure, "Couldnt partition the drive: " + restore_device_name);
                LOG_ERROR("Couldnt partition the drive: " + restore_device_name);
                return;

            }

            scf::str512 partition_path = restore_device_name;

            if (!partition_path.empty() && std::isdigit(partition_path.back())) {

                partition_path.append("p1"); 
            
            } else { 

                partition_path.append("1");

            }

            scf::println(CYAN, "[Phase 5]:", RESET);
            scf::println("mkfs.vfat -F32 " + partition_path);

            const auto mkfs_res = EXEC_QUIET_SUDO("mkfs.vfat -F32 " + partition_path);

            if (!mkfs_res.success) {

                LOG_ERROR("Failed while formatting USB: " + restore_device_name);
                ERR(ErrorCode::ProcessFailure, "Failed to format USB device with FS: " + restore_device_name);
                return;

            }

            scf::println(GREEN, "[Success] Your USB should now function as a normal FAT32 drive (partition: ", partition_path, ")", RESET);
            LOG_SUCCESS("Restored USB device " + restore_device_name + " -> formatted " + partition_path);
            return;

        } catch (const std::exception& e) {

            ERR(ErrorCode::ProcessFailure, "Failed to initialize usb restore function: " + scf::str64(e.what()));
            LOG_ERROR("failed to initialize restore usb function");
            return;

        }
    }

    // ========== Menu that took that i made in 1:51 am in the morning ===========
    enum MenuOptions {
        Burniso = 1, MountDrive = 2, UnmountDrive = 3, RESTOREUSB = 4, Exit = 0
    };

    static std::vector<std::pair<int, std::string>> getMenuItems() {
        return {
            {Burniso, "Burn iso/img to storage device"},
            {MountDrive, "Mount storage device"},
            {UnmountDrive, "Unmount storage device"},
            {RESTOREUSB, "Restore usb from iso"},
            {Exit, "Return to main menu"}
        };
    }

public:
    static void mainMountUtil() {
        printFunctionHeader("Drive Utils");

        const int menu_input = GenericMenuIO::noColorTuiMenu("Mount/Unmount", getMenuItems()); 
        
        switch (menu_input) {
            case Burniso: {
                BurnISOToStorageDevice();
                break;  
            }

            case MountDrive: {
                choose_mount_unmount("mount");
                break;
            }

            case UnmountDrive: {
                choose_mount_unmount("unmount");
                break;
            }

            case RESTOREUSB: {
                Restore_USB_Drive();
                break;
            }

            case Exit: {
                return;
            }

            default: {

                ERR(ErrorCode::OutOfRange, "Invalid menu option selected");
                return;

            }
        }
    }
};


// ========== Forensic Analysis Utilities ==========

class ForensicAnalysis {
private:
    static void CreateDiskImage() {
        try {
            scf::lnprintln_flush("[Create Disk Image]");

            const scf::str256 driveName = ListDrivesUtil::listDrives(true);

            scf::lnprintln("Enter the path where the disk image should be saved (e.g., /path/to/image.img):");
            scf::str512 imagePath = scf::read<scf::str512>();
            scf::println("Are you sure you want to create a disk image of ", driveName, " at ", imagePath, "? (y/n)");
            char confirmationcreate;
            scf::read(confirmationcreate);

            if (confirmationcreate != 'y' && confirmationcreate != 'Y') {
                scf::println("[Info] Operation cancelled");
                LOG_INFO("Operation cancelled");
                return;
            }

            auto res = EXEC_SUDO("dd if=" + driveName + " of=" + imagePath + " bs=4M status=progress && sync");
            if (!res.success) {
                ERR(ErrorCode::ProcessFailure, "Failed to create disk image");
                LOG_ERROR("Failed to create disk image for drive: " + driveName);
                return;
            }

            scf::println(GREEN, "[Success] Disk image created at ", imagePath, RESET);
            LOG_SUCCESS("Disk image created successfully for drive: " + driveName);
       
        } catch (const std::exception& e) {
            ERR(ErrorCode::ProcessFailure, "Failed to create Disk image: " + scf::str64(e.what()));
            LOG_ERROR("Failed to create disk image: " + scf::str64(e.what()));
            return;
        }
    }

    // recoverymain + side functions
    static void recovery() {
        std::vector<std::pair<int, std::string>> recovery_menu = {
            {1, "Files Recovery"},
            {2, "Partition Recovery"},
            {3, "System Recovery"},
            {0, "Return to main menu"}
        };

        int menu_choice = GenericMenuIO::noColorTuiMenu("Recovery", recovery_menu);

        switch (menu_choice) {
            case 1: {
                filerecovery();
                break;
            }

            case 2: {
                partitionrecovery();
                break;
            }

            case 3: {
                systemrecovery();
                break;
            }

            case 0: {
                return;
            }

            default: {
                ERR(ErrorCode::OutOfRange, "Invalid recovery option selected");  
                break;     
            }  
        }   
    }

    //·−−− recovery side functions
    static void filerecovery() {
        //std::string device = getAndValidateDriveName("Enter the NAME of a drive or image to scan for recoverable files (e.g., /dev/sda:");
        const std::string device = ListDrivesUtil::listDrives(true);

        static const std::vector<std::string> signature_names = {"all","png","jpg","elf","zip","pdf","mp3","mp4","wav","avi","tar.gz","conf","txt","sh","xml","html","csv"};
        std::cout << "Type signature to search (e.g. png) or 'all':\n";

        std::string sig_in;
        std::cin >> sig_in;

        size_t sig_idx = SIZE_MAX;

        for (size_t i = 0; i < signature_names.size(); ++i) if (signature_names[i] == sig_in) { sig_idx = i; break; }
        if (sig_idx == SIZE_MAX) { ERR(ErrorCode::InvalidInput, "Unsupported signature: " + sig_in); return; }

        std::cout << "Scan depth: 1=quick 2=full\n";

        auto depth = InputValidation::getInt({1, 2});
        if (!depth.has_value()) return;

        if (depth == 1) file_recovery_quick(device, (int)sig_idx);
        else if (depth == 2) file_recovery_full(device, (int)sig_idx);
    }

    static void file_recovery_quick(const std::string& drive, int signature_type) {
        std::cout << "Scanning drive for recoverable files (quick) - signature index: " << signature_type << "...\n";

        // Mapping of the numeric menu choices to signature keys
        static const std::vector<std::string> signature_names = {
            "all", "png", "jpg", "elf", "zip", "pdf", "mp3", "mp4", "wav", "avi",
            "tar.gz", "conf", "txt", "sh", "xml", "html", "csv"
        };

        if (signature_type < 0 || static_cast<size_t>(signature_type) >= signature_names.size()) {
            ERR(ErrorCode::InvalidInput, "Invalid signature type index: " + std::to_string(signature_type));
            return;
        }

        const std::string key = signature_names[signature_type];

        // Helper: scan a signature over the drive, limited to max_blocks (SIZE_MAX = full)
        auto scan_signature = [&](const file_signature& sig, size_t max_blocks) {
            if (sig.header.empty()) return;

            const size_t block_size = 4096;
            std::ifstream disk(drive, std::ios::binary);

            if (!disk.is_open()) {
                ERR(ErrorCode::DeviceNotFound, "Cannot open drive/image: " + drive);
                return;
            }

            std::vector<uint8_t> prev_tail;
            size_t offset = 0; // bytes read so far
            size_t blocks_read = 0;
            const size_t header_len = sig.header.size();

            while (disk && (max_blocks == SIZE_MAX || blocks_read < max_blocks)) {
                std::vector<char> buf(block_size);
                disk.read(buf.data(), block_size);
                std::streamsize n = disk.gcount();
                if (n <= 0) break;

                // window = prev_tail + buf
                std::vector<uint8_t> window;
                window.reserve(prev_tail.size() + static_cast<size_t>(n));
                window.insert(window.end(), prev_tail.begin(), prev_tail.end());
                window.insert(window.end(), reinterpret_cast<uint8_t*>(buf.data()), reinterpret_cast<uint8_t*>(buf.data()) + n);

                // search for header in window
                for (size_t i = 0; i + header_len <= window.size(); ++i) {
                    if (std::memcmp(window.data() + i, sig.header.data(), header_len) == 0) {
                        size_t found_offset = offset + i - prev_tail.size();
                        std::cout << "[FOUND] ." << sig.extension << " signature at offset: " << found_offset << "\n";
                    }
                }

                // keep the last (header_len - 1) bytes to handle signatures spanning blocks
                if (header_len > 1) {
                    size_t tail_len = std::min(window.size(), header_len - 1);
                    prev_tail.assign(window.end() - tail_len, window.end());
                } else {
                    prev_tail.clear();
                }

                offset += static_cast<size_t>(n);
                ++blocks_read;
            }

            disk.close();
        };

        if (key == "all") {
            // quick: limit to first N blocks per signature to stay fast
            const size_t quick_blocks = 1024; // ~4MB
            for (const auto& kv : signatures) {
                std::cout << "Quick scanning for: " << kv.first << "\n";
                scan_signature(kv.second, quick_blocks);
            }

        } else {

            auto it = signatures.find(key);
            if (it == signatures.end()) {
                ERR(ErrorCode::InvalidInput, "Signature not found: " + key);
                return;
            }
            scan_signature(it->second, 1024);
        }
    }

    static void file_recovery_full(const std::string& drive, int signature_type) {
        std::cout << "Scanning drive for recoverable files (full) - signature index: " << signature_type << "...\n";

        static const std::vector<std::string> signature_names = {
            "all", "png", "jpg", "elf", "zip", "pdf", "mp3", "mp4", "wav", "avi",
            "tar.gz", "conf", "txt", "sh", "xml", "html", "csv"
        };

        if (signature_type < 0 || static_cast<size_t>(signature_type) >= signature_names.size()) {
            ERR(ErrorCode::InvalidInput, "Invalid signature type index: " + std::to_string(signature_type));
            return;
        }

        const std::string key = signature_names[signature_type];

        auto scan_signature_full = [&](const file_signature& sig) {
            if (sig.header.empty()) return;
            const size_t block_size = 4096;
            std::ifstream disk(drive, std::ios::binary);

            if (!disk.is_open()) {
                ERR(ErrorCode::DeviceNotFound, "Cannot open drive/image: " + drive);
                return;
            }

            std::vector<uint8_t> prev_tail;
            size_t offset = 0;
            const size_t header_len = sig.header.size();

            while (disk) {
                std::vector<char> buf(block_size);
                disk.read(buf.data(), block_size);
                std::streamsize n = disk.gcount();
                if (n <= 0) break;

                std::vector<uint8_t> window;
                window.reserve(prev_tail.size() + static_cast<size_t>(n));
                window.insert(window.end(), prev_tail.begin(), prev_tail.end());
                window.insert(window.end(), reinterpret_cast<uint8_t*>(buf.data()), reinterpret_cast<uint8_t*>(buf.data()) + n);

                for (size_t i = 0; i + header_len <= window.size(); ++i) {
                    if (std::memcmp(window.data() + i, sig.header.data(), header_len) == 0) {
                        size_t found_offset = offset + i - prev_tail.size();
                        std::cout << "[FOUND] ." << sig.extension << " signature at offset: " << found_offset << "\n";
                    }
                }

                if (header_len > 1) {
                    size_t tail_len = std::min(window.size(), header_len - 1);
                    prev_tail.assign(window.end() - tail_len, window.end());
                } else {
                    prev_tail.clear();
                }

                offset += static_cast<size_t>(n);
            }

            disk.close();
        };

        if (key == "all") {
            for (const auto& kv : signatures) {
                std::cout << "Full scanning for: " << kv.first << "\n";
                scan_signature_full(kv.second);
            }

        } else {

            auto it = signatures.find(key);
            if (it == signatures.end()) {
                ERR(ErrorCode::InvalidInput, "Signature not found: " + key);
                return;
            }
            scan_signature_full(it->second);
        }
    }

    static void partitionrecovery() {
        std::string device = ListDrivesUtil::listDrives(true);

        std::cout << "\n--- Partition table (parted) ---\n";
        auto parted_res = EXEC_SUDO("parted -s " + device + " print");
        
        std::cout << "\n--- fdisk -l ---\n";
        auto fdisk_res = EXEC_SUDO("fdisk -l " + device + " 2>/dev/null || true");

        // Offer to dump partition table using sfdisk (non-destructive)
        std::cout << "Would you like to save a partition-table dump (recommended) to a file for possible restoration? (y/N): ";
        auto save_dump = InputValidation::getChar({'y', 'n'});
        if (!save_dump.has_value()) return;

        std::string dumpPath;

        if (save_dump == 'y') {
            dumpPath = device;

            // sanitize filename: replace '/' with '_'
            for (auto &c : dumpPath) if (c == '/') c = '_';
            dumpPath = "/tmp/" + dumpPath + "_sfdisk_dump.sfdisk";
            
            std::string dump_cmd = "sfdisk -d " + device + " > " + dumpPath + " 2>&1";
            auto dump_res = EXEC_SUDO(dump_cmd);

            // Check if file was created
            std::ifstream fcheck(dumpPath);
            if (fcheck) {

                std::cout << "Partition-table dump written to: " << dumpPath << "\n";
                LOG_INFO("Partition-table dump saved: " + dumpPath + " for device: " + device);
           
            } else {

                std::cout << "[Warning] Could not write partition-table dump.\n";
                LOG_INFO("Failed to write partition-table dump for device: " + device);
            }
        }

        std::cout << "\nNotes:\n";
        std::cout << " - The tool printed the partition table above. If you see missing partitions, you can try recovery tools such as 'testdisk' or restore a saved sfdisk dump with 'sudo sfdisk " << device << " < " << (dumpPath.empty() ? "<dump-file>" : dumpPath) << "'.\n";
        std::cout << " - 'testdisk' is interactive; run it manually if you want a guided recovery.\n";

        // Check for testdisk availability and offer to run guidance only
        auto testdisk_res = EXEC_QUIET_SUDO("which testdisk 2>/dev/null || true");

        if (!testdisk_res.output.empty()) {
            std::cout << "\nDetected 'testdisk' on the system. This is an interactive tool that can help recover partitions.\n";
            std::cout << "I will not run it automatically. To run it now, open a terminal and run: sudo testdisk " << device << "\n";

        } else {
            std::cout << "\n'testdisk' not found. You can install it (usually package 'testdisk') to perform interactive partition recovery.\n";
        }

        std::cout << "\nPartition recovery helper finished. Review outputs and saved dump before attempting destructive actions.\n";
    }
    
    static void systemrecovery() {
        std::string device = ListDrivesUtil::listDrives(true);

        std::cout << "\nListing partitions and filesystems for " << device << "\n";
        std::string lsblk_cmd = "lsblk -o NAME,FSTYPE,SIZE,MOUNTPOINT,LABEL -p -n " + device;
        auto lsblk_res = EXEC(lsblk_cmd);
        
        std::cout << "\nProbing for possible boot partitions (EFI and Linux root candidates)...\n";

        // Find an EFI partition (vfat with esp flag) and a Linux root (ext4/xfs/btrfs)
        auto blkid_res = EXEC_SUDO("blkid -o export " + device + "* 2>/dev/null || true");

        // Also show partition flags from parted
        auto parted_res = EXEC_SUDO("parted -s " + device + " print");

        std::cout << "\nIf you want to attempt automatic repair of the bootloader, DriveMgr will prepare a script with suggested steps (it will not run it unless you explicitly allow execution).\n";
        // Build a suggested script (dry-run by default)
        std::string scriptPath = "/tmp/drive_mgr_repair_";
        std::string sanitized = device;
        for (auto &c : sanitized) if (c == '/') c = '_';
        scriptPath += sanitized + ".sh";

        std::ofstream script(scriptPath);
        if (!script) {
            ERR(ErrorCode::ProcessFailure, "Could not create system recovery helper script at path: " + scriptPath);
            LOG_ERROR("Could not create system recovery script: " + scriptPath);
            return;
        }

        script << "#!/bin/sh\n";
        script << "# Sectr-ctl generated helper script: inspect and run manually or allow Sectr-ctl to run with explicit confirmation.\n";
        script << "# Device: " << device << "\n";
        script << "set -e\n";
        script << "echo 'This script will attempt to mount root and reinstall grub. Inspect before running.'\n";
        script << "# Example sequence (adapt to your partition layout):\n";
        script << "# 1) Mount root partition: sudo mount /dev/sdXY /mnt\n";
        script << "# 2) If EFI: sudo mount /dev/sdXZ /mnt/boot/efi\n";
        script << "# 3) Bind system dirs: sudo mount --bind /dev /mnt/dev && sudo mount --bind /proc /mnt/proc && sudo mount --bind /sys /mnt/sys\n";
        script << "# 4) chroot and reinstall grub: sudo chroot /mnt grub-install --target=x86_64-efi --efi-directory=/boot/efi --bootloader-id=ubuntu || sudo chroot /mnt grub-install /dev/sdX\n";
        script << "# 5) update-grub inside chroot: sudo chroot /mnt update-grub\n";
        script << "echo 'Script created for guidance only. Do not run without verifying paths.'\n";
        script.close();

        auto chmod_res = EXEC("chmod +x " + scriptPath);

        std::cout << "A helper script was created at: " << scriptPath << "\n";
        std::cout << "Open and inspect it. If you want Sectr-ctl to attempt to run the helper script now, type the exact phrase 'I UNDERSTAND' (all caps) to confirm: ";
        std::string confirmation;

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::getline(std::cin, confirmation);

        if (confirmation == "I UNDERSTAND") {

            std::cout << "Running helper script (requires sudo). This is destructive if incorrect.\n";
            LOG_INFO("User allowed Sectr-ctl to run system recovery helper script: " + scriptPath);
            
            auto run_res = EXEC_SUDO("sh " + scriptPath);
            std::cout << "Helper script finished. Inspect system state manually.\n";

        } else {
            std::cout << "Not running the helper script. Inspect and run manually if desired: sudo sh " << scriptPath << "\n";
        }
    }
    //−·−
public:
    static void mainForensic() {
        enum ForensicMenuOptions {
            Info = 1, CreateDisktImage = 2, ScanDrive = 3, Exit = 0
        };

        std::vector<std::pair<int, std::string>> forensic_menu = {
            {Info, "Info about the Forensic Analysis tool"},
            {CreateDisktImage, "Create a disk image of a drive"},
            // {ScanDrive, "Recover system/files/partitions..."},
            {Exit, "Return to main menu"}
        };

        int menu_choice = GenericMenuIO::noColorTuiMenu("Forensic Analysis", forensic_menu);

        switch (static_cast<ForensicMenuOptions>(menu_choice)) {
            case Info: {
                std::cout << BOLD << "\n[Info] This is a custom made forensic analysis tool for the Drive Manager\n";
                std::cout << "Its not using actual forsensic tools, but still if its finished would be fully functional\n";
                std::cout << "In development...\n" << RESET;
                break;
            }

            case CreateDisktImage: {
                CreateDiskImage();
                break;
            }

            // case ScanDrive: {
            //     recovery();
            //     break;
            // }

            case Exit: {
                break;
            }

            default: {
                ERR(ErrorCode::OutOfRange, "Invalid menu selection in forensic analysis menu");
                return;
            }
        }
    }
};


// ========== Clone Drive Utility ==========

class Clone {
    private:
        static void CloneDrive(const scf::str512 &source, const scf::str512 &target) {
            scf::lnprintln_flush("Do you want to clone data from ", source, " to ", target, "? This will overwrite all data on the target drive(n) (y/n): ");
            
            const scf::optional<char> confirmation = InputValidation::getChar({'y', 'n'});
            if (!confirmation.has_value()) return;

            if (confirmation != 'y') {

                scf::println("[Info] Operation cancelled");
                LOG_INFO("Operation cancelled");
                return;

            } else if (confirmation == 'y') {

                const auto res = EXEC_SUDO("dd if=" + source + " of=" + target + " bs=5M status=progress && sync");

                if (!res.success) {

                    LOG_ERROR("Failed to clone drive from " + source + " to " + target);
                    ERR(ErrorCode::ProcessFailure, "Failed to clone data from " + source + " to " + target);
                    return;

                }

                scf::println(GREEN, "[Success] Drive cloned from ", source, " to ", target, "\n", RESET);
                LOG_SUCCESS("Drive cloned successfully from " + source + " to " + target);
                
            }

        }

        static const scf::str256 *validateTargetDriveName(const scf::str256 &target_drive) {
            constexpr const char *valid_paths_contains[3] {
                "/mnt/", "/dev/", "/media/"
            };

            if (target_drive.empty()) {

                ERR(ErrorCode::DataUnavailable, "Target drive cannot be empty string");
                LOG_ERROR("Target drive cannot be empty string");
                return nullptr;

            }

            for (const auto& path : valid_paths_contains) {

                if (target_drive.find(path) != scf::npos) {

                    const scf::str256 *validated_disk = &target_drive;

                    return validated_disk;

                }

            }

            ERR(ErrorCode::InvalidInput, "Target drive string doesn't contain a valid drive path prefix");
            return nullptr;
        }

        
    public:
        static void mainClone() {
            printFunctionHeader("Cloning");

            scf::lnprintln("Choose a Source drive to clone the data from it:");
            const scf::str256 source_drive = ListDrivesUtil::listDrives(true);

            scf::lnprintln("Enter a Target drive/device to clone the data on to it (dont choose the same drive):");
            scf::println(YELLOW, "[WARNING]", RESET, " Make sure to choose the mount path of the target", BOLD, " (e.g., /media/target_drive)", RESET);

            auto target_drive = InputValidation::getString();
            if (!target_drive.has_value()) return;

            const scf::str256 *validated = validateTargetDriveName(*target_drive);
            if (validated == nullptr) { return; }

            const scf::str256 val_target = *validated;

            if (source_drive == val_target) {

                LOG_ERROR("Source and target drives are the same");
                dmgr_runtime_error("[ERROR] Source and target drives cannot be the same!");
                return;

            } else {

                CloneDrive(source_drive, val_target);
                return;
            }
        }
};


// ========== Log Viewer Utility ==========

static void logViewer(bool turn_off_print_f_header = false) {
    if (!turn_off_print_f_header) {
        printFunctionHeader("Log viewer");
    }

    FILE* file = fopen(Globals::log_path.c_str(), "r");

    if (file == nullptr) {

        LOG_ERROR("Unable to read log file at " + scf::to_str128(Globals::log_path));
        ERR(ErrorCode::FileNotFound, "Unable to read log file at path: " + Globals::log_path.string());

        scf::println("Please read the log file manually at: ", Globals::log_path.string());
        return;

    }

    scf::lnprintln_flush("Log file content:");

    scf::str256 line;
    const size_t line_size = line.capacity();
    bool matching = false;
    size_t first_close = scf::npos;
    size_t tag_start = scf::npos;
    char tag_id;

    // the first ] pos is always 17 because of the defaulted Logging message style
    #define CLOSING_BRAKET_POS 17

    while (fgets(line.data(), line_size, file)) {
        line.set_length(strnlen(line.data(), line_size));

        matching = false;

        first_close = line.find(']', CLOSING_BRAKET_POS); 

        if (first_close == scf::npos) { 
            ERR(ErrorCode::Undefined, "first ']' was not found in 'line'; returned npos"); 
            LOG_ERROR("first ']' was not found in 'line'; returned npos; logViewer()");
            fclose(file);
            return;
        }

        tag_start = line.find('[', first_close + 1); 

        if (tag_start == scf::npos) {
            ERR(ErrorCode::Undefined, "second '[' was not found in 'line'; returned npos");
            LOG_ERROR("second '[' was not found in 'line'; returned npos; logViewer()");
            fclose(file);
            return;
        }

        tag_id = line[tag_start + 1];

        switch (tag_id) {
            case 'E':
                if (line[tag_start + 2] == 'R') { scf::print(RED, line, RESET); }
                else if (line[tag_start + 2] == 'X') { scf::print(CYAN, line, RESET); }
                matching = true;
                break;

            case 'W':
                scf::print(YELLOW, line, RESET);
                matching = true;
                break;

            case 'D':
                scf::print(MAGENTA, line, RESET);
                matching = true;
                break;

            case 'S':
                scf::print(GREEN, line, RESET);
                matching = true;
                break;

            default:
                break;
        }

        if (!matching) {
            scf::print(line);
        }

    }

    fseek(file, 0, SEEK_END); 
    if (long size = ftell(file) > 0) {
        scf::lnprintln("Do you want to empty the log file content? (y/n):");
    
        const auto clear_loggs = InputValidation::getChar({'y', 'n'});
        if (!clear_loggs.has_value()) return;

        if (clear_loggs == 'y') { Logger::clearLoggs(Globals::log_path.c_str()); }

    } else {
        scf::println(BOLD, "[INFO] ", RESET, "Log file is emtpy");
    }

    fclose(file);

    return;
}


// ========== Configuration Editor Utility ==========
// v0.9.19.23; added fallbacks for config values if the user doesnt specify them in the config file

class ConfigValueHandeling {
    public:
        struct CONFIG_VALUES {
            scf::str16 UI_MODE = "CLI";
            scf::str16 COMPILE_MODE = "StatBin";
            scf::str16 THEME_COLOR_MODE = "RESET";
            scf::str16 SELECTION_COLOR_MODE = "RESET";
            bool DRY_RUN_MODE = false;
            bool ROOT_MODE = false;
            bool SMART_DATA = false;
        };

        static void printConfig(const CONFIG_VALUES *cfg) {
            scf::lnprintln("┌─────", BOLD, " config values ", RESET, "─────┐");
            scf::println("│ UI mode: ", cfg->UI_MODE);
            scf::println("│ Compile mode: ", cfg->COMPILE_MODE);
            scf::println("│ Dry run mode: ", cfg->DRY_RUN_MODE);
            scf::println("│ Root mode: ", cfg->ROOT_MODE);
            scf::println("│ Theme Color: ", cfg->THEME_COLOR_MODE);
            scf::println("│ Selection Color: ", cfg->SELECTION_COLOR_MODE);
            scf::println("│ Smart metadata: ", cfg->SMART_DATA);
            scf::println("└─────────────────────────┘");   
        }

        static void configEditor(CONFIG_VALUES *cfg) {
            printFunctionHeader("Config Editor");

            printConfig(cfg);

            if (Globals::config_path.empty()) {
                return;
            }            

            scf::lnprintln("Do you want to edit the config file? (y/n)");
            
            const scf::optional<char> config_edit_confirm = InputValidation::getChar({'y', 'n'}); 
            if (!config_edit_confirm.has_value()) return; 

            if (config_edit_confirm != 'y') return;

            if (!std::filesystem::exists(Globals::lume_path)) {

                ERR(ErrorCode::FileNotFound, "Lume editor not found at: " + scf::to_str512(Globals::lume_path));
                LOG_ERROR("Lume editor missing at: " + scf::to_str512(Globals::lume_path));
                return;

            }

            if (!std::filesystem::exists(Globals::config_path)) {

                ERR(ErrorCode::FileNotFound, "Config file not found at: " + scf::to_str512(Globals::config_path));
                LOG_ERROR("Config file missing at: " + scf::to_str512(Globals::config_path));
                return;

            }

            const scf::str1024 cmd = "\"" + scf::to_str256(Globals::lume_path) + "\" \"" + scf::to_str256(Globals::config_path) + "\"";

            scf::println_flush(LEAVETERMINALSCREEN);
            term.restoreTerminal();

            system(cmd.c_str());

            scf::println_flush(NEWTERMINALSCREEN);

            term.enableRawMode();
            return;      
        }

        static CONFIG_VALUES config_init() {
            CONFIG_VALUES cfg = configHandler();
            colorThemeHandler(cfg);
            return cfg;
        }

    private: 
        static void colorThemeHandler(const CONFIG_VALUES &cfg) {

            if (Globals::g_no_color) {
                Globals::g_THEME_COLOR = RESET;
                Globals::g_SELECTION_COLOR = RESET;
                return;
            }

            auto theme_color = available_colores.find(cfg.THEME_COLOR_MODE);
            if (theme_color != available_colores.end()) {
                Globals::g_THEME_COLOR = theme_color->second;
            }

            auto selection_color = available_colores.find(cfg.SELECTION_COLOR_MODE);
            if (selection_color != available_colores.end()) {
                Globals::g_SELECTION_COLOR = selection_color->second;
            }
        }

        static CONFIG_VALUES configHandler() {
            CONFIG_VALUES cfg; 

            if (Globals::g_config_src_flag == true) {

                Globals::config_path = std::filesystem::path(scf::to_std_str(Globals::g_config_src_path));

            }

            if (Globals::config_path.empty()) {

                ERR(ErrorCode::DataUnavailable, "Using default config values!");
                LOG_ERROR("config file is using defautl values, due to emtpy config");
                return cfg;

            }

            if (!std::filesystem::exists(Globals::config_path)) {

                ERR(ErrorCode::FileNotFound, "Config file not found at path: " + scf::to_str512(Globals::config_path) + ". Check if the config exists and is readable. Returning default config values.");
                LOG_ERROR("Config file not found at path: " + scf::to_str512(Globals::config_path));
                return cfg;

            }

            std::ifstream config_file(Globals::config_path);

            if (!config_file.is_open()) {

                LOG_ERROR("[Config_handler] Cannot open config file");
                ERR(ErrorCode::FileNotFound, "Cannot open config file at path: " + Globals::config_path.string() + ". Check if the config exists and is readable. Returning default config values.");
                return cfg;

            }

            std::string line;

            while (std::getline(config_file, line)) {
                if (line.empty() || line[0] == '#') { continue; }

                size_t pos = line.find('=');
                if (pos == std::string::npos) { continue; }

                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);

                key = StrUtils::trimWhiteSpace(key);
                value = StrUtils::trimWhiteSpace(value);

                if (key == "UI_MODE") cfg.UI_MODE = value;
                else if (key == "COMPILE_MODE") cfg.COMPILE_MODE = value;
                else if (key == "COLOR_THEME") cfg.THEME_COLOR_MODE = value;
                else if (key == "SELECTION_COLOR") cfg.SELECTION_COLOR_MODE = value;
                else if (key == "DRY_RUN_MODE") {
                    std::string v = StrUtils::toLowerString(value);
                    cfg.DRY_RUN_MODE = (v == "true");
                }
                else if (key == "ROOT_MODE") {
                    std::string v = StrUtils::toLowerString(value);
                    cfg.ROOT_MODE = (v == "true");
                }
                else if (key == "SMART_DATA") {
                    std::string v = StrUtils::toLowerString(value);
                    cfg.SMART_DATA = (v == "true");
                };
                
            }
            return cfg;
        }
};


// ========== Drive Fingerprinting Utility ==========
// v0.9.19.24; applyed new ERR error handling

class DriveFingerprinting {
private:
    static DriveMetadata *getMetadata(DriveMetadata *metadata, const scf::str256& drive) {
        const scf::str1024 cmd = "lsblk -o NAME,SIZE,MODEL,SERIAL,UUID -P -p " + drive; 

        const auto res = EXEC_QUIET(cmd);

        if (!res.success || res.output.empty()) { 

            ERR(ErrorCode::ProcessFailure, "The lsblk failed to deliver data");
            LOG_ERROR("lsblk failed to deliver data");
            return nullptr; 

        }

        metadata->name       = extractt("NAME", res.output);
        metadata->size       = extractt("SIZE", res.output);
        metadata->model      = extractt("MODEL", res.output);
        metadata->serial     = extractt("SERIAL", res.output);
        metadata->uuid       = extractt("UUID", res.output);

        return metadata;
    }

    /**
     * @brief fingerprinting() takes the string combined_metadata and creates a sha256 hash of the combined_metadata
     * @param combined_metadata contains the metadata of the drive to create the sha256 has
     */
    static scf::str64 fingerprinting(const scf::str2048 &combined_metadata) {
        unsigned char hash[SHA256_DIGEST_LENGTH];

        SHA256(reinterpret_cast<const unsigned char*>(combined_metadata.c_str()), combined_metadata.size(), hash);

        scf::str64 fingerprint;

        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {

            char hex[3];
            snprintf(hex, sizeof(hex), "%02x", hash[i]);
            fingerprint.append(hex);

        }

        if (fingerprint.empty()) {

            LOG_ERROR("Failed to generate fingerprint");
            ERR(ErrorCode::ProcessFailure, "Failed to generate fingerprint");
            return "";

        }

        return fingerprint;
    }


public:
    static void fingerprinting_main() {
        printFunctionHeader("Fingerprinting");
        const scf::str256 drive_name_fingerprinting = ListDrivesUtil::listDrives(true);
        DriveMetadata *metadata = new DriveMetadata;

        metadata = getMetadata(metadata, drive_name_fingerprinting);
        if (metadata == nullptr) return;

        LOG_INFO("Retrieved metadata for drive: " + drive_name_fingerprinting);

        const scf::str2048 combined_metadata =
            *metadata->name + "|" +
            *metadata->size + "|" +
            *metadata->model + "|" +
            *metadata->serial + "|" +
            *metadata->uuid;

        const scf::str256 fingerprint = fingerprinting(combined_metadata);

        LOG_INFO("Generated fingerprint for drive: " + drive_name_fingerprinting);

        scf::println(BOLD, "Fingerprint:", RESET);
        scf::println(fingerprint, "\n");

        delete metadata;
    }
};

// ========== Main Menu and Utilities ==========

static void Info(bool print_func_header_turn_off = false) {
    if (!print_func_header_turn_off) { 
        printFunctionHeader("Info");
    }
    uint8_t setw_for_version = 0;
    if (VERSION.find("_dev") != scf::npos) { setw_for_version = 84; } else { setw_for_version = 88; }
    scf::lnprintln(Globals::g_THEME_COLOR, "┌───────────────────────────────────────────────────", RESET, BOLD, " Info ", RESET, Globals::g_THEME_COLOR, "───────────────────────────────────────────────────┐", RESET);
    scf::println(Globals::g_THEME_COLOR, "│ ", RESET, "Welcome to Linux Drive Manager (DMgr / LDM) — a program for Linux to view and operate your storage devices.", Globals::g_THEME_COLOR, "│", RESET); 
    scf::println(Globals::g_THEME_COLOR, "│ ", RESET, "Warning! You should know the basics about drives so you don't lose any data.", scf::str<31>(31, ' '), Globals::g_THEME_COLOR, "│", RESET);
    scf::println(Globals::g_THEME_COLOR, "│ ", RESET, "If you find problems or have ideas, visit the GitHub page and open an issue.", scf::str<31>(31, ' '), Globals::g_THEME_COLOR, "│", RESET);
    scf::println(Globals::g_THEME_COLOR, "│ ", RESET, BOLD, "Other info:", RESET, scf::str<96>(96, ' '), Globals::g_THEME_COLOR, "│", RESET);
    scf::println(Globals::g_THEME_COLOR, "│ ", RESET, "Version: ", BOLD, VERSION, RESET, scf::str<88>(setw_for_version, ' '), Globals::g_THEME_COLOR, "│", RESET);
    scf::println(Globals::g_THEME_COLOR, "│ ", RESET, "Github: ", BOLD, "https://github.com/Dogwalker-kryt/Sectr-ctl", RESET, scf::str<56>(56, ' '), Globals::g_THEME_COLOR, "│", RESET);
    scf::println(Globals::g_THEME_COLOR, "│ ", RESET, "Author: ", BOLD, "Dogwalker-kryt", RESET, scf::str<85>(85, ' '), Globals::g_THEME_COLOR, "│", RESET);
    scf::println(Globals::g_THEME_COLOR, "└────────────────────────────────────────────────────────────────────────────────────────────────────────────┘", RESET);    
}

static void printUsage(const char* progname) {
    scf::println("Usage: ", progname, " [options]");
    scf::println(BOLD, "Options:\n", RESET ,
              "  --version, -v       Print program version\n",
              "  --help, -h          Show this help and exit\n",
              "  --dry-run, -n       Do not perform destructive operations\n",
              "  --no-color, -nc     Disable colors (may affect the main menu)\n",
              "  --no-log, -nl       Disables all logging in the current session\n",
              "  --debug, -d         Enables debug messages in current session and Test option\n",
              "  --info, -i          Show program info\n",
              "  --logs, -l          Show log file content\n",
              "  --select <device>, -sd <device>         Pre select a drive you want to use\n",
              "  --config-src <path>, -cfg-src <path>    Use a diffrent config source temporalily\n",
              "  --stand-alone, -sa  Makes sectr run standalone with no logging, config and color\n",
              "  --config, -cfg      Prints config values of the current config\n",
              "  --smart-data, -sm   Enables smart data\n",
              "  --operation         Goes directly to a specific operation without menu\n"
    );
    if (!devSuffix()) { 
        scf::println(
            "                      Available operations:\n",
            "                        --list\n",
            "                        --format\n",
            "                        --crypt\n",
            "                        --resize\n",
            "                        --health\n",
            "                        --analyze-space\n",
            "                        --overwrite\n",
            "                        --vmetadata\n",
            "                        --info\n",
            "                        --forensics\n",
            "                        --clone\n",
            "                        --partitioner"
        );
    }

    if (devSuffix()) {
        scf::println(BOLD, "Dev options:", RESET, '\n',
            " --trigger-default, -td     Trigger default case in main switch case\n",
            " --bypasssc, -bsc           Bypass secutrity confirmation key"
        );
    }
}


static void notAvilable() {
    scf::println(BOLD, "[Attention] ", RESET ,"This function is not avilable in Stand alone mode (-sa)");
}

struct arg_pair_t {
    char long_name_[32];
    char short_name_[8];
    std::function<void()> operation_;
    bool exit_after_ = false;
};

// ==================== Main Function ====================

int main(int argc, char* argv[]) {
    Globals::version = VERSION;
    Globals::version_std_str = version_str;
    ConfigValueHandeling::CONFIG_VALUES cfg{};

    { // cli cmd

        const std::unordered_map<scf::str16, std::function<void()>> cli_commands = {
            {"--list", []()         { scf::print(LEAVETERMINALSCREEN); ListDrivesUtil::listDrives(false); } },
            {"--format", []()       { term.enableTerminosInput_diableAltTerminal(); if (!checkRoot()) return; formatDrive(); } },
            {"--crypt", []()        { term.enableTerminosInput_diableAltTerminal(); if (!checkRoot()) return; USBEnDeCryptionUtils::mainUsbEnDecryption(); } }, 
            {"--resize", []()       { term.enableTerminosInput_diableAltTerminal(); if (!checkRoot()) return; resizeDrive(); } },
            {"--health", []()       { term.enableTerminosInput_diableAltTerminal(); if (!checkRoot()) return; checkDriveHealth(); } },
            {"--analyze-space", [](){ term.enableTerminosInput_diableAltTerminal(); analyzeDiskSpace(); } },
            {"--overwrite", []()    { term.enableTerminosInput_diableAltTerminal(); if (!checkRoot()) return; overwriteDriveData(); } },
            {"--metadata", []()     { term.enableTerminosInput_diableAltTerminal(); if (!checkRootMetadata()) return; MetadataReader::mainReader(); } },
            {"--forensics", []()    { term.enableTerminosInput_diableAltTerminal(); if (!checkRoot()) return; ForensicAnalysis::mainForensic(); } },
            {"--clone", []()        { term.enableTerminosInput_diableAltTerminal(); if (!checkRoot()) return; Clone::mainClone(); } },
            {"--fingerprint", []()  { term.enableTerminosInput_diableAltTerminal(); DriveFingerprinting::fingerprinting_main(); }}
        };

        const arg_pair_t arg_pairs[14] {
            {"--no-color", "-nc", [](){Globals::g_no_color = true;}, false}, {"--no-log", "-nl", [](){Globals::g_no_log = true;}, false},
            {"--smart-data", "-sm", [](){Globals::smart_data = true;}, false}, {"--debug", "-d", [](){Globals::g_debug = true;}, false},
            {"--dry-run", "-n", [](){Globals::g_dry_run = true;}, false}, {"--help", "-h", [argv0 = argv[0]](){printUsage(argv0);}, true},
            {"--config", "-cfg", [&cfg](){ cfg = ConfigValueHandeling::config_init(); const ConfigValueHandeling::CONFIG_VALUES *pcfg = &cfg; ConfigValueHandeling::printConfig(pcfg);}, true},
            {"--stand-alone", "-sa", [](){Globals::stand_alone = true; Globals::g_no_log = true; Globals::log_path = ""; Globals::config_path = ""; Globals::g_no_color = true;}, false},
            {"--trigger-default", "-td", [](){Globals::force_default_case = true;}, false},
            {"--version", "-v", [](){scf::println(VERSION);}, true},
            {"--logs", "-l", [](){logViewer(true);}, true},
            {"--info", "-i", [](){Info(true);}, true},
            {"--bypasssc", "-bsc", [](){Globals::bypass_security_code = true;}, false},
            {"--color", "-c", [](){Globals::g_no_color = false;}, false}
        };

        for (int i = 1; i < argc; ++i) {
            scf::str32 arg = argv[i];

            if (arg.empty()) {
                continue;
            }

            for (const auto& pair : arg_pairs) {
                if (arg == pair.long_name_ || arg == pair.short_name_) {
                    pair.operation_();

                    if (pair.exit_after_) {
                        return 0;
                    }

                    continue;
                }
            }

            if (arg == "--select" || arg == "-sd") {
                Globals::g_selected_drive_by_flag = true; 
                
                if (i + 1 >= argc) {
                    ERR(ErrorCode::DataUnavailable, "No path argument provided for --select");
                    LOG_ERROR("No 3rd needed argument entered");
                    exit(1);
                }

                Globals::g_selected_drive = argv[i + 1];
                i++;
                
                if (!fileExists(Globals::g_selected_drive)) {
                    ERR(ErrorCode::DeviceNotFound, "");
                    LOG_ERROR("The device: '" + Globals::g_selected_drive + "' could not be found");
                    exit(1);
                }

                continue; 
            }
            
            if (arg == "--config-src" || arg == "-cfg-src") {

                Globals::g_config_src_flag = true;

                if (i + 1 >= argc) {
                    ERR(ErrorCode::DataUnavailable, "No path argument provided for --config-src");
                    LOG_ERROR("No 3rd needed argument entered");
                    exit(1);
                }

                Globals::g_config_src_path = argv[i + 1];
                i++;

                scf::str1024 val_config_src_path = filePathHandler(Globals::g_config_src_path);

                Globals::g_config_src_path = val_config_src_path;

                if (!fileExists(Globals::g_config_src_path)) {
                    ERR(ErrorCode::FileNotFound, "Your custom config: '" + Globals::g_config_src_path + "coudnt be found");
                    LOG_ERROR("The file: '" + Globals::g_config_src_path + "' could not be found");
                    exit(1);
                } 

                continue;
            }

            auto cmd = cli_commands.find(arg.to_std_str());
            if (cmd != cli_commands.end()) {
                cmd->second();
                return 0;
            }

            continue;
        }

    } // cli cmd

    if (!Globals::stand_alone) {

        cfg = ConfigValueHandeling::config_init();
        bool dry_run_mode = cfg.DRY_RUN_MODE;
    
        if (dry_run_mode == true) {
            Globals::g_dry_run = true;
        }   
    }

    // ===== TUI =====

    scf::print(NEWTERMINALSCREEN);

    std::vector<std::pair<MenuOptionsMain, std::string>> menuItems = {
        {LISTDRIVES, "List Drives"},                            {FORMATDRIVE, "Format Drive"},                                  {ENCRYPTDECRYPTDRIVE, "Encrypt/Decrypt USB Drives"},
        {RESIZEDRIVE, "Resize Drive"},                          {CHECKDRIVEHEALTH, "Check Drive Health"},                       {ANALYZEDISKSPACE, "Analyze Disk Space"},
        {OVERWRITEDRIVEDATA, "Overwrite Drive Data"},           {VIEWMETADATA, "View Drive Metadata"},                          {VIEWINFO, "View Info/help"},
        {MOUNTUNMOUNT, "Universal Disk tool (ISO/mount/...)"},  {FORENSIC, "Forensic Analysis/Disk Image (experimental)"},      {LOGVIEW, "Log viewer"},                                
        {CLONEDRIVE, "Clone a Drive"},                          {CONFIG, "Config Editor"},                                      {FINGERPRINT, "Fingerprint Drive"},  
        {UPDATER, "Updater"},                                   {EXITPROGRAM, "Exit"}
    };

    if (Globals::g_debug || devSuffix()) {
        menuItems.insert(menuItems.end() - 1, {TESTS, "Tests"});
    }

    static const char logview_not_avilable_c[18] = "                 ";
    static const char logview_not_avilable_nc[20] = "                   ";
    static char config_not_avilable_c[15] = "              ";
    static char config_not_avilable_nc[17] = "                ";
 
    if (Globals::stand_alone) {
        for (auto &item : menuItems) {
            if (item.first == LOGVIEW) { // 19
                item.second += BOLD + " (not avilable)" + (!Globals::g_no_color ? logview_not_avilable_c : logview_not_avilable_nc)  + RESET;
            } else if (item.first == CONFIG) {
                item.second += BOLD + " (not avilable)" + (!Globals::g_no_color ? config_not_avilable_c : config_not_avilable_nc) + RESET;
            }
        }
    }

    // func* for no_color 
    using menu_renderer = uint32_t(*)(const std::vector<std::pair<MenuOptionsMain, std::string>> &menuItems);
    menu_renderer menu_render_strategy = nullptr;

    if (Globals::g_no_color == true) {
        menu_render_strategy = MainMenuIO::noColorTuiMenu;
    } else {
        menu_render_strategy = MainMenuIO::colorTuiMenu;
    } 

    term.initiateTerminosInput();

    uint32_t selected = 0;
    uint32_t menuinput = 0;

    bool running = true;
    while (running == true) {
        term.initiateTerminosInput();

        selected = menu_render_strategy(menuItems);
        menuinput = menuItems[selected].first;

        if (Globals::force_default_case) {
            menuinput = RANDOM_NUMBER;
        } 

        switch (static_cast<MenuOptionsMain>(menuinput)) {

            case LISTDRIVES: {
                printFunctionHeader("List disks");
                ListDrivesUtil::listDrives(false);
                scf::lnprintln(BOLD, "Press any key to return, '2' for advanced listing, or '3' to exit:", RESET);
                auto menuques2 = InputValidation::getInt(1, 3);
                
                if (menuques2 == 1) { continue; }
                else if (menuques2 == 2) { listpartisions(); }
                else if (menuques2 == 3) { running = false; }
                break;
            }

            case FORMATDRIVE:           { if (checkRoot()) { formatDrive(); } menuQues(running); break; }

            case ENCRYPTDECRYPTDRIVE:   { if (checkRoot()) { USBEnDeCryptionUtils::mainUsbEnDecryption(); } menuQues(running); break; } 

            case RESIZEDRIVE:           { if (checkRoot()) { resizeDrive(); } menuQues(running); break; }

            case CHECKDRIVEHEALTH:      { if (checkRoot()) { checkDriveHealth(); } menuQues(running); break; }

            case ANALYZEDISKSPACE:      { analyzeDiskSpace(); menuQues(running); break; }

            case OVERWRITEDRIVEDATA:    { if (checkRoot()) { OverwriteUtility::overwriter(); } menuQues(running); break; }

            case VIEWMETADATA:          { if (Globals::smart_data) { if (checkRootMetadata()) { MetadataReader::mainReader(); } } else { MetadataReader::mainReader(); } menuQues(running); break; }

            case VIEWINFO:              { Info(); menuQues(running); break; }

            case MOUNTUNMOUNT:          { if (checkRoot()) { MountUtility::mainMountUtil(); } menuQues(running); break; }

            case FORENSIC:              { if (checkRoot()) { ForensicAnalysis::mainForensic(); } menuQues(running); break; }

            case LOGVIEW:               { if (Globals::stand_alone) { notAvilable(); } else { logViewer(); } menuQues(running); break; }

            case CLONEDRIVE:            { if (checkRoot()) { Clone::mainClone(); } menuQues(running); break; }

            case CONFIG:                { if (Globals::stand_alone) { notAvilable(); } else { ConfigValueHandeling::CONFIG_VALUES *pcfg = &cfg; ConfigValueHandeling::configEditor(pcfg); } menuQues(running); break; }

            case FINGERPRINT:           { DriveFingerprinting::fingerprinting_main(); menuQues(running); break; }

            case UPDATER:               { if (checkRoot()) { LDMUpdater::updaterMain(); } menuQues(running); break; }

            case TESTS: { 
                // auto res = run_all_tests_internal(); 
                // print_test_summary(res);
                // menuQues(running); 
                // break; 
                // DiskMod::main();
                // auto res = EXEC_SPINNER("sleep 3 && echo $((1 + 1 + 2 + 3 * 108564 / 364 * 764759)) && sleep 3 &&  echo $((1 + 1 + 2 + 3 * 108564 / 364 * 764759)) && sleep 3");
                menuQues(running);
                break;
            }

            case EXITPROGRAM:           { running = false; break; }

            default: {
                ERR(ErrorCode::Unknown, "Invalid selection: " + scf::to_str8(menuinput) + "; How the fuck would even trigger this happen???");
                LOG_ERROR("Invalid menu selection triggered with menuinput = " + scf::to_str8(menuinput));
                menuQues(running);
                break;
            }
        }
    }
    
    scf::print(LEAVETERMINALSCREEN);
    return 0;
}
