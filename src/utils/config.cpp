//
// Created by user on 27.12.2024.
//

#include "config.h"

#include <filesystem>

config config::load_mod_config(const std::string &mod_name) {
    if(!std::filesystem::exists("japi/config")) {
        // Create the directories
        std::filesystem::create_directories("japi/config");
    }

    std::string file = "japi/config/" + mod_name + ".cfg";

    // Check if the file exists
    if(!std::filesystem::exists(file)) {
        // If not simply return an empty config
        return {file, toml::table()};
    }

    return {file, toml::parse_file(file)};
}

config config::load(const std::string &path) {
    // Check if the file exists
    if(!std::filesystem::exists(path)) {
        // Create directories
        std::filesystem::create_directories(path);
        return {path, toml::table()};
    }

    return {path, toml::parse_file(path)};
}

void config::save() {
    std::ofstream file(path);
    file << table;
    file.close();
}
