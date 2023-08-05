#include "config.h"

#include <filesystem>

ModConfig GetModConfig(std::string modGUID) {
    ModConfig config;

    config.filePath = "japi/config/" + modGUID + ".cfg";
    
    if(!std::filesystem::exists("japi/config")) {
        std::filesystem::create_directory("japi/config");
    }

    // Check if the file exists
    if(!std::filesystem::exists(config.filePath)) {
        config.table = toml::table();
        return config;
    }
    
    config.table = toml::parse_file(config.filePath);

    return config;
}

void SaveConfig(ModConfig& config) {
    std::ofstream file(config.filePath, std::ios::out | std::ios::trunc);
    file << config.table;
    file.close();
}