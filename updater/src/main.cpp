#include <iostream>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <vector>

#include "downloader.h"
#include "utils.h"
#include "logger.h"

typedef struct UpdaterFuncs {
    HMODULE japi_updater_lib;
    Version(*GetUpdaterVersion)();
    void(*LaunchUpdater)();
} UpdaterFuncs;

UpdaterFuncs LoadJAPIUpdater() {
    // Load JAPIUpdaterLib.dll using LoadLibrary
    HMODULE japi_updater_lib = LoadLibrary("JAPIUpdaterLib.dll");

    // Check if the library was loaded successfully
    if (!japi_updater_lib) {
        SetShouldLogToConsole(true);

        JFATAL("Failed to load JAPIUpdaterLib.dll! Is the file corrupted?");
        exit(1);
    }

    // Get the address of the GetUpdaterVersion function
    FARPROC get_updater_version_addr = GetProcAddress(japi_updater_lib, "GetUpdaterVersion");

    // Check if the function was found
    if(!get_updater_version_addr) {
        SetShouldLogToConsole(true);

        JFATAL("Failed to find the GetUpdaterVersion function in JAPIUpdaterLib.dll! Is the file corrupted?");
        exit(1);
    }

    // Get the address of the LaunchUpdater function
    FARPROC launch_updater_addr = GetProcAddress(japi_updater_lib, "LaunchUpdater");
    if(!launch_updater_addr) {
        SetShouldLogToConsole(true);

        JFATAL("Failed to find the LaunchUpdater function in JAPIUpdaterLib.dll! Is the file corrupted?");
        exit(1);
    }

    UpdaterFuncs funcs;
    funcs.japi_updater_lib = japi_updater_lib;
    funcs.GetUpdaterVersion = (Version(*)())get_updater_version_addr;
    funcs.LaunchUpdater = (void(*)())launch_updater_addr;

    return funcs;
}

int main() {
    Version latest = GetLatestUpdaterVersion();

    // Check if JAPIUpdaterLib.dll exists
    if (!std::filesystem::exists("JAPIUpdaterLib.dll")) {
        DownloadUpdater(latest);
    }

    // Load the updater
    UpdaterFuncs funcs = LoadJAPIUpdater();

    // Get the updater version
    Version updater_version = funcs.GetUpdaterVersion();
    
    // Compare the updater version with the current version
    if(IsBiggerVersion(latest, updater_version)) {
        // The updater is outdated, free the library and download the latest version
        FreeLibrary(funcs.japi_updater_lib);
        DownloadUpdater(latest);

        funcs = LoadJAPIUpdater();
    }

    // Launch the updater
    funcs.LaunchUpdater();

    return 0;
}