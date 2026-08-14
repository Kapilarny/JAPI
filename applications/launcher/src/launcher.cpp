//
// Created by kapil on 16.02.2026.
//

#include "launcher.h"

#include <algorithm>
#include <filesystem>

#include "binary_file.h"
#include "logger.h"
#include "process.h"
#include "defines.h"
#include "input.h"
#include "installer.h"
#include "miniz.h"

launcher::launcher() : _cfg(config::load("japi/config/launcher.toml")) {}

void launcher::run() {
    JINFO("Running launcher version %s", LAUNCHER_VERSION);

    install_japi();

    if (_cfg.get<bool>("auto_update", true)) {
        JINFO("Auto-update is enabled, checking for updates...");
        check_for_updates();
    }

    if (_cfg.get<bool>("first_launch", true)) {
        JINFO("First launch detected, setting up...");
        _cfg.set("first_launch", false);

        _cfg.set("auto_update", input::query("Do you want to enable auto-updates? (Recommended)", "Auto-Update"));

        if (input::query("Should JAPIUpdater try to cleanup old JAPI files?", "Legacy")) {
            cleanup_old_files();
        }

        install_japi();
    }

    launch_game();
}

void launcher::check_for_updates() {

}

void launcher::install_japi() {
    // TODO: Grab release package from the internet and verify it

    binary_file update_file("update420.japi", true);

    installer inst(update_file);
    inst.install();
}

void launcher::cleanup_old_files() {
    std::vector<std::string> old_files = {
        "d3dcompiler_47_o.dll",
        "JAPIPreload.dll",
        "dinput8.dll",
        "dinput8_o.dll",
        "JAPIInstaller.exe"
    };

    // Check if any of the old files exist
    if (!std::ranges::any_of(old_files, [](const std::string& file) { return std::filesystem::exists(file); })) {
        return;
    }

    JINFO("Detected old files...");

    // Prompt the user to delete them
    const int result = MessageBoxA(nullptr, "Detected old JAPI files. Do you want to delete them? (Recommended)", "Old Files Detected", MB_ICONQUESTION | MB_YESNO);
    if (result != IDYES) {
        JINFO("User chose not to delete old files, skipping cleanup...");
        return;
    }

    // Old japi shit
    if (std::filesystem::exists("d3dcompiler_47_o.dll")) {
        // Delete the old JAPI if it exists
        if (std::filesystem::exists("d3dcompiler_47_o.dll")) {
            std::filesystem::remove("d3dcompiler_47.dll");
            JINFO("Deleted old file: d3dcompiler_47_o.dll");
        }

        std::filesystem::rename("d3dcompiler_47_o.dll", "d3dcompiler_47.dll");
        JINFO("Restored d3dcompiler_47.dll");
    }

    // Add configs to the list of old files to delete
    old_files.emplace_back("japi/config/JAPI.cfg");
    old_files.emplace_back("japi/config/updater.cfg");

    for (const auto& file : old_files) {
        if (std::filesystem::exists(file)) {
            std::filesystem::remove(file);
            JINFO("Deleted old file: %s", file.c_str());
        }
    }
}

void launcher::launch_game() {
    // Get current PWD
#ifdef DEBUG_MODE
    const std::string current_path = R"(C:\Program Files (x86)\Steam\steamapps\common\JoJo's Bizarre Adventure All-Star Battle R)";
#else
    const std::string current_path = std::filesystem::current_path().string();
#endif

    const std::string game_path = current_path + "\\ASBR.exe";

    process g_process(game_path.c_str(), current_path.c_str());
    do {
        g_process.restart();
        g_process.inject_dll(std::string(current_path + R"(\japi\dlls\JAPIPreload.dll)").c_str());
        g_process.resume(true);
    } while (g_process.get_exit_code() == 67);
}