//
// Created by kapil on 1.12.2025.
//

#include "config.h"

#include <filesystem>

config config::load_mod_config(const std::string &mod_name) {
    const std::string file = "japi/config/" + mod_name + ".cfg";

    return load(file);
}

config config::load(const std::string &path) {
    // Check if the file exists
    if(!std::filesystem::exists(path)) {
        // Create directories up to the last slash
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());

        return {path, toml::table()};
    }

    return {path, toml::parse_file(path)};
}

void config::save() const {
    std::ofstream file(path);
    file << table;
    file.close();
}
