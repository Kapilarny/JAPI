#include <iostream>
#include <fstream>
#include <windows.h>
#include <vector>

#include "downloader.h"
#include "cppcrc.h"
#include "config.h"
#include "utils.h"
#include "logger.h"

#include "updater.h"

void SaveConfig(toml::table& config) {
    std::ofstream config_file("japi/config/updater.cfg", std::ios::out | std::ios::trunc);
    config_file << config;
}

int UpdaterMain() {
    // Check if japi/ exists
    if (!std::filesystem::exists("japi")) {
        std::filesystem::create_directory("japi");
    }

    // Check if japi/config/ exists
    if (!std::filesystem::exists("japi/config")) {
        std::filesystem::create_directory("japi/config");
    }

    bool is_connections = std::filesystem::exists("NSUNSC.exe");

    toml::table updater_config;
    bool should_update = false;
    bool ignore_hashes = false;

    // Check if japi/config/updater.toml exists
    if(!std::filesystem::exists("japi/config/updater.cfg")) {
        JINFO("japi/config/updater.cfg is missing! Creating a new one...");
        updater_config = toml::table();
    } else {
        updater_config = toml::parse_file("japi/config/updater.cfg");
    }

    bool open_console = ConfigBind(updater_config, "open_console", true);
    bool first_run = ConfigBind(updater_config, "first_run", true);

    SetShouldLogToConsole(open_console);

    if(open_console) {
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    }

    if(first_run) {
        int auto_update = MessageBoxA(NULL, "Do you want to enable auto-updates?", "JojoAPI Updater", MB_YESNO | MB_ICONQUESTION);
        should_update = auto_update == IDYES;

        updater_config.insert_or_assign("autoupdate", should_update);

        updater_config.insert_or_assign("first_run", false);
    }

    should_update = ConfigBind(updater_config, "autoupdate", true);
    ignore_hashes = ConfigBind(updater_config, "ignore_hashes", false);

    std::string japi_version_installed = ConfigBind(updater_config, "version", "0.0.0");
    uint16_t asbr_hash = ConfigBind(updater_config, "asbr_hash", 0);

    // Save the config
    SaveConfig(updater_config);

    if(first_run) {
        // Create the steam_appid.txt file
        CreateSteamAppID();

        if(!is_connections) {
            // Get the new hash
            uint16_t new_hash = DownloadASBR();

            if(new_hash == 0) {
                JFATAL("Failed to download a new ASBR release! Is this hash correct? (" + std::to_string(new_hash) + ")");
                JFATAL("Aborting...");

                updater_config.insert_or_assign("first_run", true);

                SaveConfig(updater_config);

                return 1;
            }

            // Insert the hash into the table
            updater_config.insert_or_assign("asbr_hash", new_hash);
        }

        DownloadJAPI(GetLatestJAPIVersion());
        DownloadAdditionalDLLs();

        updater_config.insert_or_assign("version", VersionString(GetLatestJAPIVersion()));

        // Save the config.
        SaveConfig(updater_config);

        JINFO("First run complete!");
        LaunchGame();

        return 0;
    }

    if(!should_update) {
        JINFO("Skipping updating process...");

        LaunchGame();

        return 0;
    }
 
    // Check the JAPI version
    Version latest_version = GetLatestJAPIVersion();

    if(IsBiggerVersion(latest_version, ParseVersion(japi_version_installed)) && !ignore_hashes) {
        JDEBUG("A new version of JAPI is available! Downloading...");

        DownloadJAPI(latest_version);
        DownloadAdditionalDLLs();

        // Insert the hash into the table
        updater_config.insert_or_assign("version", VersionString(latest_version));
    }

    if(!is_connections) {
        std::ifstream asbr_file("ASBR.exe");
        if(!asbr_file.good()) {
            JFATAL("ASBR.exe is missing! Is this the right directory? ABORTING");
            return 1;
        }

        // Check the ASBR.exe hash
        uint16_t computed_hash = ComputeCRC16Hash(asbr_file);

        if(computed_hash != asbr_hash && !ignore_hashes) {
            JDEBUG("ASBR.exe failed the current checksum! Grabbing the new executable...");

            // Get the new hash
            uint16_t new_hash = DownloadASBR();

            if(new_hash == 0) {
                JERROR("Failed to download a new ASBR release! Is this hash correct? (" + std::to_string(new_hash) + ")");
                JERROR("Skipping updating process...");

                LaunchGame();

                return 0;
            }

            // Insert the hash into the table
            updater_config.insert_or_assign("asbr_hash", new_hash);
        }
    }

    // Save the config
    SaveConfig(updater_config);
    LaunchGame();

    return 0;
}