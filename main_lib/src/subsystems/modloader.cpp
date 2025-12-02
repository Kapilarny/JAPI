//
// Created by kapil on 1.12.2025.
//

#include "modloader.h"

#include <filesystem>
#include <fstream>

#include "JoJoAPI.h"
#include "logger.h"

void modloader::post_init() {
    load_mods();
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
                JINFO("Skipping disabled plugin: %s", e.path().string().c_str());
                continue;
            }

            JINFO("Found plugin: %s", e.path().string().c_str());
            load_mod(e.path().string().c_str());
        }
    }
}

void modloader::load_mod(const char* file) {
    HMODULE mod = LoadLibraryA(file);
    if (mod == nullptr) {
        JERROR("Failed to load plugin: %s", file);
        return;
    }

    auto get_mod_meta = (JAPIModMeta(__stdcall*)())GetProcAddress(mod, "GetModMeta");
    if (get_mod_meta == nullptr) {
        JERROR("Mod %s does not have a GetModMeta function", file);
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
