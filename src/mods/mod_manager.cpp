//
// Created by user on 27.12.2024.
//

#include "mod_manager.h"

#include <filesystem>
#include <imgui.h>
#include <thread>
#include <unordered_set>

#include "japi.h"
#include "events/event.h"
#include "exports/JoJoAPI.h"
#include "utils/downloader.h"
#include "utils/logger.h"
#include "utils/reloader.h"

#define MODLOADER_GUID "DllPluginLoader"

std::mutex download_mutex; // protects `is_downloading` and `currently_downloading`
bool is_downloading = false;
std::string currently_downloading;

bool manifest_event_handler(void* e) {
    // Load the mod manifest
    mod_manager::get_instance()->load_mod_manifest();

    // If should download default plugins, download them
    if(japi::get_instance().should_download_default_plugins()) {
        mod_manager::get_instance()->download_default_plugins();
    }

    return false;
}

void mod_manager::download_default_plugins() {
    // Grab all the default plugins
    const auto& default_plugins = manifest["default_plugins"][game_type_to_string(japi::get_instance().get_game_type())];

    const auto& mod_array = manifest["games"][game_type_to_string(japi::get_instance().get_game_type())];

    // Download the default plugins
    for(const auto& entry : default_plugins) {
        // Try to find the proper entry
        const auto it = std::find_if(mod_array.begin(), mod_array.end(), [&entry](const nlohmann::json& mod) {
            return mod["guid"] == entry;
        });

        auto proper_entry = *it;
        std::string guid = proper_entry["guid"];
        std::string name = proper_entry["name"];

        if(loaded_mods_by_guid.contains(guid) || downloaded_mods.contains(guid)) continue;

        // Download the mod
        download_mod_with_deps_sync(proper_entry, guid, name);
    }

    Sleep(1000); // wait a second for a good measure

    // Reboot the game
    reloader::reload_game();
}

void mod_manager::init() {
    instance = std::unique_ptr<mod_manager>(new mod_manager());

    instance->load_mods();
    // instance->load_mod_manifest();

    // Register mod manifest loading for JAPILateInitEvent
    event_transmitter::register_callback("JAPILateInitEvent", manifest_event_handler);
}

void mod_manager::download_manifest() {
    // Download the manifest file
    JINFO("Downloading manifest file...");

    std::vector<uint8_t> buffer = downloader::download_file("http://raw.githubusercontent.com/JoJosBizarreModdingCommunity/JAPI_PluginRepository/main/manifest.json");
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

uint64_t mod_manager::get_latest_manifest_version() {
    std::vector<uint8_t> buffer = downloader::download_file("http://raw.githubusercontent.com/JoJosBizarreModdingCommunity/JAPI_PluginRepository/main/manifest_version.txt");
    if(buffer.empty()) {
        LOG_ERROR(MODLOADER_GUID, "Failed to download manifest version file");
        return 0;
    }

    return std::stoull(std::string(buffer.begin(), buffer.end()));
}

void mod_manager::parse_manifest() {
    std::ifstream manifest_file("japi/manifest.json");
    if (!manifest_file.is_open()) {
        LOG_ERROR(MODLOADER_GUID, "Failed to open manifest file");
        return;
    }

    // Parse the manifest file
    try {
        manifest = nlohmann::json::parse(manifest_file);
    } catch (const nlohmann::json::parse_error& e) {
        LOG_ERROR(MODLOADER_GUID, "Failed to parse manifest file: %s", e.what());
        return;
    }

    manifest_file.close();
}

void mod_manager::load_mod_manifest() {
    if(!std::filesystem::exists("japi/manifest.json")) {
        download_manifest();
    }

    parse_manifest();

    // Check if the manifest version is latest
    auto grabbed_version = get_latest_manifest_version();
    LOG_INFO(MODLOADER_GUID, "Latest manifest version: %lld", grabbed_version);
    if(manifest["manifest_version"].get<int>() < grabbed_version) {
        LOG_WARN(MODLOADER_GUID, "Manifest version is outdated (current: %d, latest: %d)", manifest["manifest_version"].get<int>(), grabbed_version);

        // Download the latest manifest
        download_manifest();
        parse_manifest();
    }

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

        instance->lazy_load_mod(mod, mod_guid.c_str());
    }
}

// manifest.json
/*
 * {
 *   "manifest_version": 1,
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

void mod_manager::download_mod_with_deps_sync(const nlohmann::basic_json<> &entry, const std::string& guid, const std::string& name) {
    std::vector<std::pair<std::string, HMODULE>> loaded_mods;

    for(const auto& dependency : entry["dependencies"]) {
        if(loaded_mods_by_guid.contains(dependency)) continue;

        // Get the dependency entry
        const auto& mod_array = manifest["games"][game_type_to_string(japi::get_instance().get_game_type())];

        // Find the dependency entry
        const auto it = std::find_if(mod_array.begin(), mod_array.end(), [&dependency](const nlohmann::json& entry) {
            return entry["guid"] == dependency;
        });

        // If the dependency entry is not found, skip it
        if(it == mod_array.end()) {
            LOG_ERROR(MODLOADER_GUID, "Failed to find dependency %s", dependency.get<std::string>().c_str());
            continue;
        }

        // Download the dependency
        HMODULE mod;
        if(!grab_and_load_mod_from_manifest(*it, &mod)) {
            LOG_ERROR(MODLOADER_GUID, "Failed to download mod %s", dependency.get<std::string>().c_str());
        }

        // Add the mod to the loaded mods
        if((*it)["lazy_load"]) loaded_mods.emplace_back(dependency, mod);
    }

    // Load the main mod
    HMODULE mod;
    if(!grab_and_load_mod_from_manifest(entry, &mod)) {
        LOG_ERROR(MODLOADER_GUID, "Failed to download mod %s", name.c_str());
    }

    if(entry["lazy_load"]) loaded_mods.emplace_back(guid, mod);

    // Lazy-load everything
    for(const auto& [guid, mod] : loaded_mods) {
        instance->lazy_load_mod(mod, guid.c_str());
    }

    LOG_INFO(MODLOADER_GUID, "Successfully downloaded the plugin! (You may need to reload the game to load it)");
}

void mod_manager::download_mod_with_deps_async(const nlohmann::basic_json<> &entry, const std::string& guid, const std::string& name) {
    // Set the `is_downloading` flag
    {
        std::lock_guard<std::mutex> lock(download_mutex);
        is_downloading = true;
    }

    // Create a thread for downloading
    std::thread([&]() {
        download_mod_with_deps_sync(entry, guid, name);

        // Unset the `is_downloading` flag
        {
            std::lock_guard<std::mutex> lock(download_mutex);
            is_downloading = false;
        }
    }).detach();
}

void mod_manager::draw_imgui_mods_tab() {
    {
        // Try to access `is_downloading` && `currently_downloading`
        std::lock_guard<std::mutex> lock(download_mutex);

        if(is_downloading) {
            ImGui::Text("Downloading: %s", currently_downloading.c_str());
            return;
        }
    }

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

            if(loaded_mods_by_guid.contains(guid) || downloaded_mods.contains(guid)) continue;

            // Get name
            const std::string name = entry["name"];

            if(ImGui::TreeNode(name.c_str())) {
                ImGui::Text("Author: %s", entry["author"].get<std::string>().c_str());
                ImGui::Text("Version: %s", entry["version"].get<std::string>().c_str());
                ImGui::Text("Description: %s", entry["description"].get<std::string>().c_str());
                ImGui::TextLinkOpenURL("Source Code", entry["source_url"].get<std::string>().c_str());

                if(ImGui::Button("Download (with dependencies)")) {
                    download_mod_with_deps_async(entry, guid, name);
                }

                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }

    if(ImGui::Button("Reload Game")) {
        reloader::reload_game();
    }
}

void mod_manager::lazy_load_mod(HMODULE mod, const char *mod_guid) {
    // Get the mod meta
    auto get_mod_meta = (JAPIModMeta(__stdcall*)())GetProcAddress(mod, "GetModMeta");
    if (get_mod_meta == nullptr) {
        LOG_ERROR(MODLOADER_GUID, "Mod %s does not have a GetModMeta function", mod_guid);
        FreeLibrary(mod);
        return;
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
    loaded_mods[mod] = meta;
    loaded_mods_by_guid[retr_meta.guid] = mod;

    // Call the mod init function
    auto mod_init = (void(__stdcall*)())GetProcAddress(mod, "ModInit");
    if (mod_init != nullptr) {
        mod_init();
    }
}

bool mod_manager::grab_and_load_mod_from_manifest(const nlohmann::basic_json<> &entry, HMODULE* out_mod) {
    // Download the mod
    std::string guid = entry["guid"];
    std::string name = entry["name"];
    std::string url = entry["url"];

    // Check if the mod is already downloaded
    if(downloaded_mods.contains(guid)) {
        LOG_TRACE(MODLOADER_GUID, "Mod %s is already downloaded", name.c_str());
        return false;
    }

    {
        // Set the `currently_downloading` variable
        std::lock_guard<std::mutex> lock(download_mutex);
        currently_downloading = name;
    }

    std::vector<uint8_t> buffer = downloader::download_file(url);
    if(buffer.empty()) {
        LOG_ERROR(MODLOADER_GUID, "Failed to download mod %s", name.c_str());
        ImGui::OpenPopup("Download Failed");
    }

    // Write the buffer to the file
    std::filesystem::create_directories("japi/plugins");
    std::ofstream file("japi/plugins/" + guid + ".dll", std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR(MODLOADER_GUID, "Failed to open file for writing: %s", (guid + ".dll").c_str());
        ImGui::OpenPopup("Download Failed");
    }

    // Write the buffer to the file
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    file.close();

    downloaded_mods.insert(guid); // Add the mod to the downloaded mods

    HMODULE mod = LoadLibraryA(("japi/plugins/" + guid + ".dll").c_str());
    if (mod == nullptr) {
        LOG_ERROR(MODLOADER_GUID, "Failed to load plugin %s", (guid + ".dll").c_str());
        return false;
    }

    *out_mod = mod;

    return true;
}