#include "japi.h"

#include <Windows.h>
#include <filesystem>
#include <MinHook.h>

#include "utils/logger.h"
#include "exports/JojoAPI.h"
#include "lua/script.h"

void JAPI::Init(HINSTANCE hinstDLL) {
    instance = std::unique_ptr<JAPI>(new JAPI());
    instance->hinstDLL = hinstDLL;

    instance->asbrModuleBase = (uint64_t) GetModuleHandle(NULL);

    if(MH_Initialize() != MH_OK) {
        JERROR("Failed to initialize MinHook!");
        return;
    }

    ScriptManager::Init();

    JINFO("Initialized JojoAPI!");
    
    instance->LoadMods();

    // For now janky loop
    while(true) {
        ScriptManager::ExecuteScripts();
        Sleep(1000);
    }
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

    if(!std::filesystem::exists("japi\\luamods")) {
        std::filesystem::create_directory("japi\\luamods");
    }

    // Get all files in japi\mods and load them
    for(auto& p : std::filesystem::directory_iterator("japi\\mods")) {
        if(p.path().extension() == ".dll") {
            // Load the DLL
            auto handle = LoadLibrary(p.path().string().c_str());
            if(!handle) {
                LOG_ERROR("ModLoader", "Failed to load mod " + p.path().string());
                continue;
            }

            // Get the mod info
            auto getModInfo = (ModMeta (*)()) GetProcAddress(handle, "GetModInfo");
            if(!getModInfo) {
                LOG_ERROR("ModLoader", "Failed to get mod info for " + p.path().string());
                continue;
            }

            ModMeta modInfo = getModInfo();
            auto modInit = (void (*)()) GetProcAddress(handle, "ModInit");
            if(!modInit) {
                LOG_ERROR("ModLoader", "Failed to get mod init for " + p.path().string());
                continue;
            }

            modInit();

            mods.push_back({modInfo, handle});
        }
    }

    // Get all files in japi\luamods and load them
    for(auto& p : std::filesystem::directory_iterator("japi\\luamods")) {
        if(p.path().extension() == ".lua") {
            LOG_INFO("ModLoader", "Loading Lua mod " + p.path().string());
            ScriptManager::AddFileToWatch(p.path().string());
        }
    }

    LOG_INFO("ModLoader", "Loaded " + std::to_string(mods.size()) + " mods!");
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