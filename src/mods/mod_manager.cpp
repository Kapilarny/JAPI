//
// Created by user on 27.12.2024.
//

#include "mod_manager.h"

#include <filesystem>
#include <imgui.h>
#include <unordered_set>

#include "exports/JoJoAPI.h"
#include "utils/logger.h"

#define MODLOADER_GUID "DllPluginLoader"

void mod_manager::init() {
    instance = std::unique_ptr<mod_manager>(new mod_manager());

    instance->load_mods();
}

void mod_manager::load_mods() {
    LOG_INFO(MODLOADER_GUID, "Loading mods...");

    if(!std::filesystem::exists("japi/dll-plugins")) {
        std::filesystem::create_directories("japi/dll-plugins");
    }

    // Find all mod DLLs from the directory
    std::unordered_set<std::string> mod_files; // filename with extension
    for (const auto& entry : std::filesystem::directory_iterator("japi/dll-plugins")) {
        if (entry.is_regular_file() && entry.path().extension() == ".dll") {
            // Check if the first character of the filename is a `-`
            if (entry.path().filename().string().front() == '-') {
                LOG_TRACE(MODLOADER_GUID, "Skipping mod %s", entry.path().filename().string().c_str());
                continue;
            }

            mod_files.insert(entry.path().filename().string());
        }
    }

    std::unordered_map<std::string, HMODULE> loaded_mods;

    // Load the mods
    for(const auto& mod_file : mod_files) {
        HMODULE mod = LoadLibraryA(("japi/dll-plugins/" + mod_file).c_str());
        if (mod == nullptr) {
            LOG_ERROR(MODLOADER_GUID, "Failed to load mod %s", mod_file.c_str());
            continue;
        }

        loaded_mods[mod_file] = mod;
    }

    // Initialize the mods
    for(const auto& [mod_guid, mod] : loaded_mods) {
        // Check if all dependencies are met
        auto get_mod_dependencies = (JAPIModDependencies(__stdcall*)())GetProcAddress(mod, "GetModDependencies");
        if (get_mod_dependencies != nullptr) {
            JAPIModDependencies dependencies = get_mod_dependencies();

            bool dependencies_met = true;
            for(int i = 0; i < dependencies.dependencies_count; i++) {
                const char* dependency = dependencies.dependencies[i];
                if (loaded_mods.find(dependency) == loaded_mods.end()) {
                    LOG_ERROR(MODLOADER_GUID, "Mod %s has an unmet dependency: %s", mod_guid.c_str(), dependency);
                    FreeLibrary(mod);
                    dependencies_met = false;
                    break;
                }
            }

            if(!dependencies_met) {
                continue;
            }
        }

        // Get the mod meta
        auto get_mod_meta = (JAPIModMeta(__stdcall*)())GetProcAddress(mod, "GetModMeta");
        if (get_mod_meta == nullptr) {
            LOG_ERROR(MODLOADER_GUID, "Mod %s does not have a GetModMeta function", mod_guid.c_str());
            FreeLibrary(mod);
            continue;
        }

        // Create the dll_mod_meta struct
        auto retr_meta = get_mod_meta();

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
        instance->loaded_mods[mod] = meta;

        // Call the mod init function
        auto mod_init = (void(__stdcall*)())GetProcAddress(mod, "ModInit");
        if (mod_init != nullptr) {
            mod_init();
        }
    }
}

void mod_manager::draw_imgui_menu() {
    for(auto const& [mod, meta] : loaded_mods) {
        if(ImGui::CollapsingHeader(meta.name)) {
            ImGui::Text("Author: %s", meta.author);
            ImGui::Text("Version: %s", meta.version);
            ImGui::Text("Description: %s", meta.description);

            if(meta.draw_imgui_func != nullptr) {
                ImGui::Separator();

                meta.draw_imgui_func();
            }
        }
    }
}
