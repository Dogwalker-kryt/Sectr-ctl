#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include "scf/scf.hpp"

namespace fs = std::filesystem;

struct PartInfo {
    scf::str128 name;
    double size_MiB_d;
    scf::str128 size_str;
    scf::str64 type;
    scf::str128 mountpoint;
    scf::str16 fs_type;
};

struct DiskInfo {
    std::string device;
    std::string size;
    std::string type;
    std::string mount;
    std::string fstype;
    std::string status;
    size_t partition_count;
    std::vector<PartInfo> partitions;
};
struct Row {
    std::string device, size, type, mount, fstype, status;
};

class DiskLister {
private:
    std::vector<DiskInfo> disks;

    std::string readFile(const std::string& path);
    std::string getMountPoint(const std::string& device);
    std::string getFsType(const std::string& device);

public:
    DiskLister() = default;
    
    /**
     * @brief Fetches all physical drives info from /sys/block/
     * @return vector of DiskInfo with device data
     */
    std::vector<DiskInfo> getPhysicalDisksInfo();

    /**
     * @brief Same as getPhysicalDisksInfo but returns as Row structs for UI
     * @return vector of Row
     */
    std::vector<Row> getPhysicalDisksInfoAsRows();

    /**
     * @brief Get cached disks info (after getPhysicalDisksInfo was called)
     * @return const reference to disks vector
     */
    const std::vector<DiskInfo>& getCachedDisks() const;

    /**
     * @brief Refresh disk information (re-scan /sys/block/)
     */
    void refresh();
};