//
// Created by user on 27.12.2024.
//

#ifndef MOD_MANAGER_H
#define MOD_MANAGER_H
#include <memory>
#include <unordered_map>

#include <Windows.h>

#include "utils/config.h"

#include <json.hpp>

// void DrawImGUI();
typedef void (__stdcall* DllModDrawImGUI)();

struct dll_mod_meta {
    const char* name{};
    const char* author{};
    const char* guid{};
    const char* version{};
    const char* description{};

    config mod_config = config::load_mod_config("Unknown"); // Default to unknown
    DllModDrawImGUI draw_imgui_func = nullptr;
};

class mod_manager {
public:
    static void init();

    void draw_imgui_mods_tab();
    void load_mod_manifest();

    static mod_manager* get_instance() { return instance.get(); }

    dll_mod_meta* get_mod_meta(HMODULE mod) {
        // Lookup the module
        const auto it = loaded_mods.find(mod);
        if (it == loaded_mods.end()) {
            return nullptr;
        }

        return &it->second;
    }
private:
    mod_manager() = default;

    void load_mods();
    void lazy_load_mod(HMODULE mod, const char* mod_guid);
    bool grab_and_load_mod_from_manifest(const nlohmann::basic_json<>& entry, HMODULE* out_mod);
    void download_manifest();
    void parse_manifest();


    uint64_t get_latest_manifest_version();

    static inline std::unique_ptr<mod_manager> instance;

    std::unordered_map<HMODULE, dll_mod_meta> loaded_mods;
    std::unordered_map<std::string, HMODULE> loaded_mods_by_guid;
    nlohmann::json manifest;
    bool manifest_loaded = false;
};


#endif //MOD_MANAGER_H
