#include <iostream>
#include <fstream>
#include <windows.h>
#include <vector>

#include "downloader.h"
#include "cppcrc.h"
#include "config.h"
#include "utils.h"

int main() {
    // Create the console window
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    // Check if japi/ exists
    if (!std::filesystem::exists("japi")) {
        std::filesystem::create_directory("japi");
    }

    // Check if japi/config/ exists
    if (!std::filesystem::exists("japi/config")) {
        std::filesystem::create_directory("japi/config");
    }

    toml::table updater_config;
    bool should_update = true;
    bool ignore_hashes = false;
    uint32_t japi_version_installed = 0;
    uint16_t asbr_hash = 0;
    uint16_t japi_hash = 0;

    // Check if japi/config/updater.toml exists
    if(!std::filesystem::exists("japi/config/updater.toml")) {
        std::ofstream updater_config_file("japi/config/updater.toml", std::ios::out | std::ios::trunc);

        updater_config = toml::table();
        should_update = ConfigBind(updater_config, "autoupdate", true);
        ignore_hashes = ConfigBind(updater_config, "ignore_hashes", false);
        japi_version_installed = ConfigBind(updater_config, "version", 0);
        asbr_hash = ConfigBind(updater_config, "asbr_hash", 0);
        japi_hash = ConfigBind(updater_config, "japi_hash", 0);

        updater_config_file << updater_config;
    }

    if(!japi_version_installed || !asbr_hash) {
        // TODO: Install ASBR.exe and d3dcompiler_47.dll
    }

    // Check the ASBR.exe hash
    std::ifstream asbr_o("ASBR.exe");
    if(!asbr_o.good()) {
        // TODO: Install ASBR.exe
    }

    uint16_t computed_hash = ComputeCRC16Hash(asbr_o);
    if(computed_hash != asbr_hash) {
        std::cout << "ASBR.exe failed the current checksum! Grabbing the executable...\n";

        // TODO: Install ASBR.exe
    }

    // Check the d3dcompiler_47.dll hash
    std::ifstream d3dcompiler_47("d3dcompiler_47.dll");
    if(!d3dcompiler_47.good()) {
        // TODO: Install d3dcompiler_47.dll & d3dcompiler_47_o.dll
    }

    computed_hash = ComputeCRC16Hash(d3dcompiler_47);

    if(computed_hash != japi_hash) {
        std::cout << "JAPI failed the current checksum! Grabbing newest release...\n";

        // TODO: Install d3dcompiler_47.dll & d3dcompiler_47_o.dll
    }

    return 0;
}