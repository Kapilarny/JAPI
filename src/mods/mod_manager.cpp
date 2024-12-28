//
// Created by user on 27.12.2024.
//

#include "mod_manager.h"

#include <filesystem>
#include <imgui.h>
#include <unordered_set>

#include "japi.h"
#include "events/event.h"
#include "exports/JoJoAPI.h"
#include "utils/downloader.h"
#include "utils/logger.h"

#define MODLOADER_GUID "DllPluginLoader"

bool manifest_event_handler(void* e) {
    // Load the mod manifest
    mod_manager::get_instance()->load_mod_manifest();

    return false;
}

void mod_manager::init() {
    instance = std::unique_ptr<mod_manager>(new mod_manager());

    instance->load_mods();
    // instance->load_mod_manifest();

    // Register mod manifest loading for JAPILateInitEvent
    event_transmitter::register_callback("JAPILateInitEvent", manifest_event_handler);
}

void mod_manager::load_mod_manifest() {
    if(!std::filesystem::exists("japi/manifest.json")) {
        // Download the manifest file
        JINFO("Downloading manifest file...");

        std::vector<uint8_t> buffer = downloader::download_file("http://raw.githubusercontent.com/Kapilarny/JAPI/master/manifest.json");
        if(buffer.empty()) {
            LOG_ERROR(MODLOADER_GUID, "Failed to download manifest file");
            return;
        }

        // Write the buffer to the file
        std::ofstream manifest_file("japi/manifest.json", std::ios::binary);
        if (!manifest_file.is_open()) {
            LOG_ERROR(MODLOADER_GUID, "Failed to open manifest file for writing");
            return;
        }

        // Write the buffer to the file
        manifest_file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        manifest_file.close();
    }

    std::ifstream manifest_file("japi/manifest.json");
    if (!manifest_file.is_open()) {
        LOG_ERROR(MODLOADER_GUID, "Failed to open manifest file");
        return;
    }

    manifest_file >> manifest;

    manifest_loaded = true;
}

void mod_manager::load_mods() {
    LOG_INFO(MODLOADER_GUID, "Loading mods...");

    if(!std::filesystem::exists("japi/plugins")) {
        std::filesystem::create_directories("japi/plugins");
    }

    // Find all mod DLLs from the directory
    std::unordered_set<std::string> mod_files; // filename with extension
    for (const auto& entry : std::filesystem::directory_iterator("japi/plugins")) {
        if (entry.is_regular_file() && entry.path().extension() == ".dll") {
            // Check if the first character of the filename is a `-`
            if (entry.path().filename().string().front() == '-') {
                LOG_TRACE(MODLOADER_GUID, "Skipping plugin %s", entry.path().filename().string().c_str());
                continue;
            }

            mod_files.insert(entry.path().filename().string());
        }
    }

    std::unordered_map<std::string, HMODULE> loaded_mods;

    // Load the mods
    for(const auto& mod_file : mod_files) {
        HMODULE mod = LoadLibraryA(("japi/plugins/" + mod_file).c_str());
        if (mod == nullptr) {
            LOG_ERROR(MODLOADER_GUID, "Failed to load plugin %s", mod_file.c_str());
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
        instance->loaded_mods_by_guid[retr_meta.guid] = mod;

        // Call the mod init function
        auto mod_init = (void(__stdcall*)())GetProcAddress(mod, "ModInit");
        if (mod_init != nullptr) {
            mod_init();
        }
    }
}

// manifest.json
/*
 * {
 *   "manifest_version": "1.0.0",
 *   "games": {
 *      "ASBR": [
 *          {
 *              "name": "Test Mod",
 *              "guid": "TestMod",
 *              "author": "Kapilarny"
 *              "version": "1.0.0",
 *              "description": "This is a test mod",
 *              "url": "...",
 *              "dependencies": ["AnotherMod"]
 *          }
 *      ],
 *      "NSUNSC": [...]
 *   }
 * }
 */

void mod_manager::draw_imgui_mods_tab() {
    if(ImGui::TreeNode("Loaded Plugins")) {
        for(auto const& [mod, meta] : loaded_mods) {
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

        ImGui::TreePop();
    }

    if(!manifest_loaded) return; // Don't draw the available plugins if the manifest isn't loaded yet

    if(ImGui::TreeNode("Available Plugins")) {
        for(const auto& entry : manifest["games"][game_type_to_string(japi::get_instance().get_game_type())]) {
            // Get guid
            const std::string guid = entry["guid"];

            if(loaded_mods_by_guid.contains(guid)) continue;

            // Get name
            const std::string name = entry["name"];

            if(ImGui::TreeNode(name.c_str())) {
                ImGui::Text("Author: %s", entry["author"].get<std::string>().c_str());
                ImGui::Text("Version: %s", entry["version"].get<std::string>().c_str());
                ImGui::Text("Description: %s", entry["description"].get<std::string>().c_str());

                if(ImGui::Button("Download (with dependencies)")) {
                    // TODO: Implement mod downloading
                    JINFO("Downloading mod %s", name.c_str());
                }

                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }
}
