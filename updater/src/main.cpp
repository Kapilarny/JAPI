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
    bool first_run = false;
    bool delete_eac = false;
    bool ignore_hashes = false;
    std::string japi_version_installed = "";
    uint16_t asbr_hash = 0;

    // Check if japi/config/updater.toml exists
    if(!std::filesystem::exists("japi/config/updater.toml")) {
        std::cout << "japi/config/updater.toml is missing! Creating a new one...\n";
        updater_config = toml::table();
    } else {
        updater_config = toml::parse("japi/config/updater.toml");
    }

    std::ofstream updater_config_file("japi/config/updater.toml", std::ios::out | std::ios::trunc);

    first_run = ConfigBind(updater_config, "first_run", true);

    if(first_run) {
        int auto_update = MessageBoxA(NULL, "Do you want to enable auto-updates?", "JojoAPI Updater", MB_YESNO | MB_ICONQUESTION);
        should_update = auto_update == IDYES;

        updater_config.insert_or_assign("autoupdate", should_update);

        int should_remove_eac = MessageBoxA(NULL, "Do you want uninstall EAC?", "JojoAPI Updater", MB_YESNO | MB_ICONQUESTION);
        delete_eac = should_remove_eac == IDYES;

        updater_config.insert_or_assign("delete_eac", delete_eac);

        updater_config.insert_or_assign("first_run", false);
    }

    should_update = ConfigBind(updater_config, "autoupdate", true);
    delete_eac = ConfigBind(updater_config, "delete_eac", true);
    ignore_hashes = ConfigBind(updater_config, "ignore_hashes", false);
    japi_version_installed = ConfigBind(updater_config, "version", "0.0.0");
    asbr_hash = ConfigBind(updater_config, "asbr_hash", 0);

    // Save the config
    updater_config_file << updater_config;

    if(first_run) {
        // Get the new hash
        uint16_t new_hash = DownloadASBR();

        if(new_hash == 0) {
            std::cout << "Failed to download a new ASBR release! Is this hash correct? (" + std::to_string(new_hash) + ")\n";
            std::cout << "Aborting...\n";

            updater_config.insert_or_assign("first_run", true);

            updater_config_file << updater_config;

            return 1;
        }

        // Insert the hash into the table
        updater_config.insert_or_assign("asbr_hash", new_hash);

        DownloadJAPI(GetLatestJAPIVersion());
        DownloadAdditionalDLLs();

        if(delete_eac) {
            RemoveEAC();
        }

        std::cout << "First run complete!\n";
        LaunchGame();

        // Save the config
        updater_config_file << updater_config;

        return 0;
    }

    if(!should_update) {
        std::cout << "Skipping updating process...\n";

        LaunchGame();

        return 0;
    }

    std::ifstream asbr_file("ASBR.exe");
    if(!asbr_file.good()) {
        std::cout << "ASBR.exe is missing! Is this the right directory? ABORTING\n";
        return 1;
    }

    // Check the ASBR.exe hash
    uint16_t computed_hash = ComputeCRC16Hash(asbr_file);

    if(computed_hash != asbr_hash) {
        std::cout << "ASBR.exe failed the current checksum! Grabbing the new executable...\n";

        // Get the new hash
        uint16_t new_hash = DownloadASBR();

        if(new_hash == 0) {
            std::cout << "Failed to download a new ASBR release! Is this hash correct? (" + std::to_string(new_hash) + ")\n";
            std::cout << "Skipping updating process...\n";

            LaunchGame();

            return 0;
        }

        // Insert the hash into the table
        updater_config.insert_or_assign("asbr_hash", new_hash);
    }

    // Check the JAPI version
    Version latest_version = GetLatestJAPIVersion();

    if(IsBiggerVersion(latest_version, ParseVersion(japi_version_installed)) && !ignore_hashes) {
        std::cout << "A new version of JAPI is available! Downloading...\n";

        DownloadJAPI(latest_version);
        DownloadAdditionalDLLs();

        // Insert the hash into the table
        updater_config.insert_or_assign("version", VersionString(latest_version));
    }

    // Save the config
    updater_config_file << updater_config;

    LaunchGame();

    return 0;
}