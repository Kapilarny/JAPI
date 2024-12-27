//
// Created by user on 27.12.2024.
//

#ifndef MOD_MANAGER_H
#define MOD_MANAGER_H
#include <memory>
#include <unordered_map>

#include <Windows.h>

#include "utils/config.h"

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

    void draw_imgui_menu();

    static mod_manager* get_instance() { return instance.get(); }

    dll_mod_meta* get_mod_meta(HMODULE mod) {
        // Lookup the module
        const auto it = loaded_mods.find(mod);
        if (it == loaded_mods.end()) {
            return nullptr;
        }

        return &it->second;
    }

    HMODULE get_mod_hmodule(const std::string& guid) {
        // Lookup the module
        const auto it = guid_to_mod.find(guid);
        if (it == guid_to_mod.end()) {
            return nullptr;
        }

        return it->second;
    }
private:
    mod_manager() = default;

    void load_mods();

    static inline std::unique_ptr<mod_manager> instance;
    std::unordered_map<HMODULE, dll_mod_meta> loaded_mods;
    std::unordered_map<std::string, HMODULE> guid_to_mod;
};


#endif //MOD_MANAGER_H
