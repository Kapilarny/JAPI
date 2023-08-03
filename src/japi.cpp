#include "japi.h"

#include <Windows.h>
#include <filesystem>

#include "exports/JojoAPI.h"

void JAPI::Init(HINSTANCE hinstDLL) {
    instance = std::unique_ptr<JAPI>(new JAPI());
    instance->hinstDLL = hinstDLL;

    instance->asbrModuleBase = (uint64_t) GetModuleHandle(NULL);

    printf("[JojoApi]: Initialized!\n");
    instance->LoadMods();
}

uint64_t JAPI::GetASBRModuleBase() {
    return instance->asbrModuleBase;
}

void JAPI::LoadMods() {
    if(!std::filesystem::exists("japi")) {
        std::filesystem::create_directory("japi");
    }

    if(!std::filesystem::exists("japi\\mods")) {
        std::filesystem::create_directory("japi\\mods");
    }

    // Get all files in japi\mods and load them
    for(auto& p : std::filesystem::directory_iterator("japi\\mods")) {
        if(p.path().extension() == ".dll") {
            // Load the DLL
            auto handle = LoadLibrary(p.path().string().c_str());
            if(!handle) {
                printf("[ModLoader]: Failed to load mod %s\n", p.path().string().c_str());
                continue;
            }

            // Get the mod info
            auto getModInfo = (ModMeta (*)()) GetProcAddress(handle, "GetModInfo");
            if(!getModInfo) {
                printf("[ModLoader]: Failed to get mod info for %s\n", p.path().string().c_str());
                continue;
            }

            ModMeta modInfo = getModInfo();
            auto modInit = (void (*)()) GetProcAddress(handle, "ModInit");
            if(!modInit) {
                printf("[ModLoader]: Failed to get mod init for %s\n", p.path().string().c_str());
                continue;
            }

            modInit();

            mods.push_back({modInfo, handle});
        }
    }

    printf("[ModLoader]: Loaded %d mods\n", mods.size());
}

std::string JAPI::GetModGUID(HANDLE modHandle) {
    for(auto& mod : instance->mods) {
        if(mod.handle == modHandle) {
            return mod.meta.guid;
        }
    }

    return "";
}

HANDLE JAPI::GetASBRProcessHandle() {
    if(!instance->processHandle) {
        instance->InitWindowsVars();
    }

    return instance->processHandle;
}

void JAPI::InitWindowsVars() {
    if(!instance->hwnd) {
        instance->hwnd = FindWindowA(NULL, "Jojo's Bizarre Adventure: All-Star Battle R");
        if(!instance->hwnd) {
            Sleep(5);
            InitWindowsVars(); // This is stupid but it works so idc
            return;
        }
    }

    if(!instance->pid) {
        GetWindowThreadProcessId(instance->hwnd, &instance->pid);
    }

    if(!instance->processHandle) {
        instance->processHandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, instance->pid);
    }
}