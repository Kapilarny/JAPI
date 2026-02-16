//
// Created by kapil on 16.02.2026.
//

#include "launcher.h"

#include "logger.h"
#include "process.h"
#include "defines.h"

void launcher::run() {
    JINFO("Starting launcher version %s", LAUNCHER_VERSION);
    launch_game();
}

void launcher::launch_game() {
    // Get current PWD
#ifdef DEBUG_MODE
    const std::string current_path = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\JoJo's Bizarre Adventure All-Star Battle R";
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
