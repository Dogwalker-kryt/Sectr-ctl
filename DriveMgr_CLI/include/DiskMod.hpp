#pragma once

#include "DmgrLib.h"
#include "cmd_exec/exec_cmd.h"
#include "scf/scf.hpp"
#include "ui/ListDrivesUtil.hpp"
#include "DiskLister.hpp"
#include <iomanip>
#include <sstream>
#include <string>

namespace DiskMod {
struct DiskOpeartion {
    DiskInfo disk;
    uint64_t operation_code;
    int8_t escape_cpde;
};

enum DiskOperation_escape_code {
    SUCCESS = 0, ERROR = -1  
};



inline scf::str128 formatGB(double gb) {
    std::ostringstream oss;

    if (gb >= 1000.0) {
        oss << std::fixed << std::setprecision(1) << gb / 1000.0 << " TB";
    } else {
        oss << std::fixed << std::setprecision(0) << gb << " GB";
    }

    return scf::to_str128(oss.str());
}

inline double convertSizeToMiB(const std::string &size_str) {
    if (size_str.empty() || size_str == "-") return 0.0;
    
    size_t i = 0;
    while (i < size_str.length() && (std::isdigit(size_str[i]) || size_str[i] == '.')) {
        i++;
    }
    
    double value = std::stod(size_str.substr(0, i));
    std::string unit = size_str.substr(i);
    
    if (unit.empty() || unit == "B") {
        return value / 1'000'000;  
    } else if (unit == "K") {
        return value / 1'000;      
    } else if (unit == "M") {
        return value;              
    } else if (unit == "G") {
        return value * 1'000;      
    } else if (unit == "T") {
        return value * 1'000'000;  
    }
    
    return 0.0;  
}

inline std::vector<PartInfo> getPartitionInfoAboutDisk(const scf::str512 &disk_name) {
    std::vector<PartInfo> partition_info_vec;
    
    const str1024 cmd = "lsblk -o NAME,SIZE,TYPE,MOUNTPOINT,FSTYPE -n -p " + disk_name; 
    const auto res = EXEC(cmd);

    if (!res.success) {
        ERR(ErrorCode::ProcessFailure, "lsblk failed");
        LOG_ERROR("lsblk failed for disk: " + disk_name);
        return partition_info_vec;  
    }

    std::istringstream iss(res.output);
    std::string line;

    while (std::getline(iss, line)) {
        if (line.find("part") == std::string::npos) continue;

        std::istringstream lss(line);
        std::string part_name, part_size, part_type, part_mount, part_fstype;
        
        lss >> part_name >> part_size >> part_type;
        
        std::string rest;
        std::getline(lss, rest);
        std::istringstream rss(rest);
        rss >> part_mount >> part_fstype;
        
        PartInfo p;
        
        size_t part_name_len = part_name.length();
        p.name = part_name.substr(6, part_name_len);
        
        double part_size_d = std::stod(part_size);

        // Parse size string to double (e.g., "100G" -> 100.0)
        p.size_MiB_d = convertSizeToMiB(part_size); 
        p.size_str = formatGB(part_size_d);
        
        p.type = part_type;
        p.mountpoint = (part_mount == "-") ? "" : part_mount;
        p.fs_type = (part_fstype == "-") ? "" : part_fstype;
        
        partition_info_vec.push_back(p);
    }

    return partition_info_vec;
}


inline DiskInfo populateDiskInfo(const scf::str512 &disk_name, DiskLister lister) {
    DiskInfo disk_DiskInfo;
    
    DiskInfo disk{};
    const std::vector<DiskInfo> &all_disks_DiskInfo = lister.getPhysicalDisksInfo();

    for (const auto& d : all_disks_DiskInfo) {
        if (d.device == scf::to_std_str(disk_name)) {
            disk = d;
            break;
        }
    }

    std::vector<PartInfo> part_info_vec = getPartitionInfoAboutDisk(disk.device);

    disk_DiskInfo.device = disk.device;
    disk_DiskInfo.size = disk.size;
    disk_DiskInfo.fstype = disk.fstype;
    disk_DiskInfo.mount = disk.mount;
    disk_DiskInfo.partition_count = part_info_vec.size();
    disk_DiskInfo.partitions = part_info_vec;
    disk_DiskInfo.status = disk.status;
    disk_DiskInfo.type = disk.type;

    return disk_DiskInfo;
}

class Menu {
private:
    static inline size_t drawMenu(DiskInfo &__disk__) {
        size_t selected = 0;
        const size_t total_elements = __disk__.partitions.size();

        if (total_elements == 0) {
            return static_cast<size_t>(-1); 
        }

        system("clear");
        scf::println("────────────────────────── ", BOLD, "Disk Mod", RESET, " ──────────────────────────");
        scf::lnprintln(" disk: ", __disk__.device);
        scf::println(" size: ", formatGB(std::stod(__disk__.size.substr(0, __disk__.size.length() - 1))));
        scf::lnprintln("──────────────────────────────────────────────────────────────");
        std::cout << "#  " 
          << std::left 
          << std::setw(15) << "part"
          << std::setw(15) << "size"
          << std::setw(15) << "type"
          << std::setw(22) << "mountpoint"
          << std::setw(12) << "fs type" 
        << "\n\n";

       
        while (true) {
            for (size_t i = 0; i < total_elements; ++i) {
                const PartInfo &p = __disk__.partitions[i];
                scf::print("\r");

                if (i == selected) {
                    scf::print(INVERSE, ">> ");
                } else {
                    scf::print("   ");
                }

                // Format: #  part_name  size  type  mountpoint  fs type
                std::cout << std::left
                  << std::setw(15) << scf::to_std_str(p.name)
                  << std::setw(15) << scf::to_std_str(p.size_str)
                  << std::setw(15) << scf::to_std_str(p.type)
                  << std::setw(22) << scf::to_std_str(p.mountpoint)
                  << std::setw(12) << scf::to_std_str(p.fs_type)
                  << RESET << "\n";
            }

            scf::flush_stdout();

            std::cout << "\033[" << total_elements << "A";

            char c;
            if (read(STDIN_FILENO, &c, 1) <= 0) continue;

            if (c == '\x1b') {
                char seq[2];
                if (read(STDIN_FILENO, &seq, 2) == 2) {
                    if (seq[1] == 'A') { // Up arrow
                        selected = (selected - 1 + total_elements) % total_elements;
                    } else if (seq[1] == 'B') { // Down arrow
                        selected = (selected + 1) % total_elements;
                    }
                }
            } else if (c == '\n' || c == '\r') { // Enter
                break;
            } 
        }

        std::cout << "\033[" << total_elements << "B";
        return selected;
    }

public:
    static inline DiskOpeartion menuMain(DiskInfo &__disk__) {
        system("clear");
        size_t choosen_disk_part = drawMenu(__disk__);

        if (choosen_disk_part == static_cast<size_t>(-1)) {
            ERR(ErrorCode::DataUnavailable, "");
            LOG_ERROR("No partitoin got returned in drawMenu()");
            return { __disk__, 255, -1 };
        }



        return { __disk__, 0, 0};
    }

};



inline void main() {
    printFunctionHeader("Disk modefier");
    const scf::str512 disk_name = ListDrivesUtil::listDrives(true);
    
    scf::println(YELLOW, "[Warning] ", RESET, "Do you really want to modify the partitions and formating of ", BOLD, "'", disk_name, "'", RESET, " (y/n)" );

    auto validation = InputValidation::getChar({'y', 'n'});
    if (!validation.has_value()) return;

    if (validation.value() == 'n' && validation.value() != 'y') {

        scf::println(BOLD, "[Aborted] ", RESET, "The operation has been aborted");
        LOG_INFO("DiskMod on " + disk_name + " has been aborted");
        return;

    }

    term.enableRawMode();

    DiskLister lister;
    DiskInfo disk;
    
    const std::vector<DiskInfo> &all_disks = lister.getPhysicalDisksInfo();

    for (const auto& d : all_disks) {
        if (d.device == scf::to_std_str(disk_name)) {
            disk = d;
            break;
        }
    }

    disk = populateDiskInfo(disk_name, lister);

    DiskOpeartion diskopt = Menu::menuMain(disk);

    term.restoreTerminal();
    return;
}


}