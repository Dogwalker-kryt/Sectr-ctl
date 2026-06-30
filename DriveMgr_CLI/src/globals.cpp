#include "../include/globals.h"

// === color related globals ===

bool Globals::g_no_color = false;

scf::str8 Globals::g_THEME_COLOR = "";

scf::str8 Globals::g_SELECTION_COLOR = "";


// === drive related globals ===

scf::str512 Globals::g_selected_drive;

bool Globals::g_selected_drive_by_flag = false;

std::vector<scf::str512> Globals::g_last_drives;


// === config related globals ===


bool Globals::g_config_src_flag = false;

scf::str1024 Globals::g_config_src_path;


// === program state globals ===

bool Globals::stand_alone = false;
bool Globals::g_dry_run = false;
bool Globals::g_no_log = false;
bool Globals::g_debug =  false;
bool Globals::smart_data = false;

std::filesystem::path Globals::dmgr_root = EnvSys::appRoot();
std::filesystem::path Globals::log_path = dmgr_root / "data" / "log.dat";
std::filesystem::path Globals::config_path = dmgr_root / "data" / "config.conf";
std::filesystem::path Globals::lume_path = dmgr_root / "bin" / "Lume" / "Lume";