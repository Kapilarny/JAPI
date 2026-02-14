//
// Created by kapil on 12.02.2026.
//
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "binary_file.h"
#include "crypt.h"
#include "downloader.h"
#include "logger.h"
#include "process.h"

#define DEBUG_MODE

int main() {
    // Get current PWD
#ifdef DEBUG_MODE
    const std::string current_path = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\JoJo's Bizarre Adventure All-Star Battle R";
#else
    const std::string current_path = std::filesystem::current_path().string();
#endif

    const std::string game_path = current_path + "\\ASBR.exe";

    // Init logger
    logger::init("JAPILauncher", "japi/logs");

    try {
        process g_process(game_path.c_str(), current_path.c_str());
        JINFO("Spawned process successfully!");

        // binary_file test_signed("test.txt.signed", true);
        // if (test_signed.verify_signature()) {
        //     JINFO("Verified signature of test.txt.signed successfully!");
        // } else {
        //     JERROR("Failed to verify signature of test.txt.signed!");
        // }

        // downloader dl;
        // dl.download_file("https://nbg1-speed.hetzner.com/100MB.bin"); // Test download to ensure downloader is working

        g_process.inject_dll(std::string(current_path + R"(\japi\dlls\JAPIPreload.dll)").c_str());
        JINFO("Injected JAPIPreload...");

        JINFO("Resuming the game process...");
        g_process.resume(true);

        while (g_process.get_exit_code() == 67) {
            JINFO("Restarting the game process...");
            g_process.restart();
            g_process.inject_dll(std::string(current_path + R"(\japi\dlls\JAPIPreload.dll)").c_str());
            g_process.resume(true);
        }
    } catch (const std::exception& e) {
        ERROR_AND_QUIT(e.what());
    }

    JINFO("Shutting down launcher...");
}
