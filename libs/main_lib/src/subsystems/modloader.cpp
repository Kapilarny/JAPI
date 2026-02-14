//
// Created by kapil on 1.12.2025.
//

#include "modloader.h"

#include <filesystem>
#include <fstream>
#include <ranges>

#include "gui.h"
#include "imgui.h"
#include "japi.h"
#include "JoJoAPI.h"
#include "logger.h"

void modloader::post_init() {
    load_mods();

    japi::get().register_post_init_callback([this] {
        gui_manager::get().register_tab_item("Loaded Plugins", [this] {
            for (const auto &meta : loaded_mods | std::views::values) {
                if(ImGui::TreeNode(meta.name)) {
                    ImGui::Text("Author: %s", meta.author);
                    ImGui::Text("Version: %s", meta.version);
                    ImGui::Text("Description: %s", meta.description);

                    if(meta.draw_imgui_func != nullptr) {
                        ImGui::Separator();

                        meta.draw_imgui_func();
                    }

                    ImGui::TreePop();
                }
            }
        });
    });
}

std::string modloader::get_mod_reserved_directory(const std::string &mod_guid) {
    if (!std::filesystem::exists("japi/plugins/" + mod_guid)) {
        std::filesystem::create_directories("japi/plugins/" + mod_guid);
    }
    return "japi/plugins/" + mod_guid + "/";
}

void modloader::load_mods() {
    JINFO("Loading plugins...");

    if (!std::filesystem::exists("japi/plugins")) {
        std::filesystem::create_directories("japi/plugins");
    }

    // Find all .dll files in the plugins directory
    for (const auto& e : std::filesystem::directory_iterator("japi/plugins"))  {
        if (e.path().extension() == ".dll") {
            if (!std::filesystem::is_regular_file(e.path())) {
                continue;
            }

            if (e.path().stem().string()[0] == '-') {
                JWARN("Skipping disabled plugin: %s", e.path().string().c_str());
                continue;
            }

            JTRACE("Found plugin: %s", e.path().string().c_str());
            load_mod("japi/plugins/", e.path().stem().string().c_str());
        }
    }
}

void modloader::load_mod_dependencies(const char *path, const char *filename) {
    // Check if the mod contains a dependencies.conf file
    std::string dep_file_path = std::string(path) + filename + "/dependencies.conf";
    if (!std::filesystem::exists(dep_file_path)) {
        return;
    }

    JTRACE("Trying to load dependencies for mod: %s", filename);

    // Load the file
    config dep_config = config::load(dep_file_path);

    // Recursively try to load dependencies
    toml::array* dependencies = dep_config.get_data()["dependencies"].as_array();
    if (!dependencies) {
        JWARN("Mod %s has a `dependencies.conf` file but it does not contain a valid `dependencies` array", filename);
        return;
    }

    // Iterate over dependencies
    for (const auto& dep : *dependencies) {
        if (dep.type() != toml::node_type::string) {
            JWARN("Mod %s has an invalid dependency entry (not a string), skipping", filename);
            continue;
        }

        std::string dep_name = dep.as_string()->get();
        if (loaded_mods_by_guid.contains(dep_name)) continue;

        if (load_chain_set.contains(dep_name)) {
            JERROR("Cyclic dependency detected when loading mod %s: already loading dependency %s", filename, dep_name.c_str());
            continue;
        }

        JTRACE("Loading dependency %s for mod %s", dep_name.c_str(), filename);
        load_mod(path, dep_name.c_str());
    }
}

void modloader::load_mod(const char* path, const char* filename) {
    if (loaded_mods_by_guid.contains(filename)) {
        JTRACE("Mod %s is already loaded, skipping", filename);
        return;
    }

    load_chain_set.insert(filename);
    load_mod_dependencies(path, filename);

    std::string full_path = std::string(path) + filename + ".dll";
    HMODULE mod = LoadLibraryA(full_path.c_str());
    if (mod == nullptr) {
        JERROR("Failed to load plugin: %s", filename);
        return;
    }

    auto get_mod_meta = (JAPIModMeta(__stdcall*)())GetProcAddress(mod, "GetModMeta");
    if (get_mod_meta == nullptr) {
        JERROR("Mod %s does not have a GetModMeta function", filename);
        FreeLibrary(mod);
        return;
    }

    // Create the dll_mod_meta struct
    auto retr_meta = get_mod_meta();

    if (strcmp(retr_meta.guid, filename) != 0) {
        JERROR("Plugin %s has mismatched GUID in GetModMeta: %s", filename, retr_meta.guid);
        FreeLibrary(mod);
        return;
    }

    dll_mod_meta meta = {
        .name = retr_meta.name,
        .author = retr_meta.author,
        .guid = retr_meta.guid,
        .version = retr_meta.version,
        .description = retr_meta.description,
        .mod_config = config::load_mod_config(retr_meta.guid)
    };

    auto draw_imgui = (DllModDrawImGUI)GetProcAddress(mod, "DrawImGUI");
    if (draw_imgui != nullptr) {
        meta.draw_imgui_func = draw_imgui;
    }

    // Add the mod to the loaded mods
    loaded_mods[mod] = meta;
    loaded_mods_by_guid[retr_meta.guid] = mod;

    // Call the mod init function
    auto mod_init = (void(__stdcall*)())GetProcAddress(mod, "ModInit");
    if (mod_init != nullptr) {
        mod_init();
    }
}

void dll_mod_meta::log() {
    JTRACE("Mod Meta - Name: %s, Author: %s, GUID: %s, Version: %s, Description: %s", name, author, guid, version, description);
}

dll_mod_meta* modloader::get_mod_meta(HMODULE module) {
    return &loaded_mods[module];
}

modloader::~modloader() {
    // Free preloaded libraries
    for (const HMODULE lib : preloaded_libs) {
        FreeLibrary(lib);
    }
}

void modloader::init() {
    if (instance != nullptr) {
        JERROR("Trying to reinitialize modloader!");
        return;
    }

    instance = std::unique_ptr<modloader>(new modloader());
    instance->post_init();
}

modloader& modloader::get() {
    if (instance == nullptr) {
        JFATAL("Trying to access modloader before initialization!");
    }

    return *instance;
}
