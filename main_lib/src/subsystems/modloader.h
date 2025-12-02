//
// Created by kapil on 1.12.2025.
//

#ifndef JAPI_PRELOAD_MODLOADER_H
#define JAPI_PRELOAD_MODLOADER_H

#include <string>
#include <unordered_map>
#include <windows.h>
#include <vector>

#include "config.h"

// void DrawImGUI();
typedef void (__stdcall* DllModDrawImGUI)();

struct dll_mod_meta {
    const char* name{};
    const char* author{};
    const char* guid{};
    const char* version{};
    const char* description{};

    config mod_config = config::load_mod_config("Unknown");
    DllModDrawImGUI draw_imgui_func = nullptr;
};

class modloader {
public:
    ~modloader();

    static void init();
    static modloader& get();

    void load_mods();

    dll_mod_meta* get_mod_meta(HMODULE module);
private:
    modloader() = default;

    void post_init();
    void load_mod(const char* file);

    // void preload_libraries();
    // void preload_library(const std::string& path);

    std::vector<HMODULE> preloaded_libs;
    std::unordered_map<HMODULE, dll_mod_meta> loaded_mods;
    std::unordered_map<std::string, HMODULE> loaded_mods_by_guid;

    static inline std::unique_ptr<modloader> instance = nullptr;
};

#endif //JAPI_PRELOAD_MODLOADER_H