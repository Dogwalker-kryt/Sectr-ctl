#include "../include/LDM_updater.h"



LDMUpdater::Version_int LDMUpdater::parseVersionVals(const str16 &v) {
    Version_int ver;
    str16 ver_str = v;

    if (!ver_str.empty() && (ver_str[0] == 'v' || ver_str[0] == 'V')) {
        ver_str.erase(0, 1);
     }

    size_t dash_pos = ver_str.find('-');
    if (dash_pos != scf::str_t::npos) {
        ver_str.substr(dash_pos);
    }

    std::stringstream ss(ver_str.to_std_str<16>());
    char dot;
    ss >> ver.major_realease >> dot
        >> ver.major >> dot 
        >> ver.minor >> dot 
        >> ver.patch;
    return ver;
}

int LDMUpdater::comparing_versions(const Version_int &version_local, const Version_int &version_github) {
    if (version_local.major_realease != version_github.major_realease) {
        return (version_local.major_realease < version_github.major_realease) ? -1 : 1;
    }
    if (version_local.major != version_github.major) {
        return (version_local.major < version_github.major) ? -1 : 1;
    }
    if (version_local.minor != version_github.minor) {
        return (version_local.minor < version_github.minor) ? -1 : 1;
    }
    if (version_local.patch != version_github.patch) {
        return (version_local.patch < version_github.patch) ? -1 : 1;
    }
    return 0;
}

str64 LDMUpdater::getVersionGithub() {
    str1024 cmd = "curl -s https://api.github.com/repos/Dogwalker-kryt/Sectr-ctl/releases/latest";
    auto res = EXEC_QUIET_SUDO(cmd);
    str4096 json = res.output;

    size_t pos = json.find("\"tag_name\"");
    if (pos == scf::str_t::npos) {
        return "";
    }

    pos = json.find(":", pos);
    pos = json.find("\"", pos) + 1;
    size_t end = json.find("\"", pos);
    size_t version_length = end - pos;
    return json.substr(pos, version_length);
}

void LDMUpdater::updaterMain(const str16 &LOCAL_VERSION) {
    std::cout << YELLOW << "\n[INFO] " << RESET << "Make sure you are connected to the internet\n";

    std::string dev_suffix;

    if (LOCAL_VERSION.find("dev") != std::string::npos || LOCAL_VERSION.find("dev") != std::string::npos) {
        dev_suffix = YELLOW + "\n[INFO]" + RESET + "Local version contains " + BOLD + "'dev'." + RESET + " Local version is a developer/custom/other release build\n";    
    }

    Version_int local_version = parseVersionVals(LOCAL_VERSION);
    str64 remote_version_str = getVersionGithub();
    Version_int remote_version = parseVersionVals(remote_version_str);

    int cmp = comparing_versions(local_version, remote_version);

    if (cmp < 0) {
        std::cout << YELLOW << "\n[UPDATE] " << RESET << "New version available: " << remote_version_str << "\n";
        std::cout
            << "If you'd like to update, you can visit the GitHub releases page: "
            << BLUE << "https://github.com/Dogwalker-kryt/Sectr-ctl/releases" << RESET << "\n"
            << "Just reinstall the latest version (you may want to back up your config file first).\n"
            << "If you're happy with your current version, feel free to keep using it — no pressure at all.\n";

    } else if (cmp > 0) {
        std::cout << YELLOW << "\n[INFO]" << RESET << "Local version is newer (probably dev build or custom build)\n";

        if (!dev_suffix.empty()) {
            std::cout << dev_suffix;
        }

    } else {
        std::cout << GREEN << "\n[INFO]" << RESET << "You are up to date.\n";

        if (!dev_suffix.empty()) {
            std::cout << dev_suffix;
        }
    }
}

