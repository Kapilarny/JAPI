#include "japi.h"

#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#include <filesystem>
#include <MinHook.h>

#include "utils/logger.h"
#include "utils/cpk.h"
#include "exports/JojoAPI.h"
#include "lua/script.h"
#include "lua/asm.h"

void JAPI::Init(HINSTANCE hinstDLL) {
    instance = std::unique_ptr<JAPI>(new JAPI());
    instance->hinstDLL = hinstDLL;
    instance->asbrModuleBase = (uint64_t) GetModuleHandle(NULL);

    // Load config
    instance->japiConfig = GetModConfig("JAPI");
    bool shouldSpawnConsole = ConfigBind<bool>(instance->japiConfig.table, "spawn_console", true);
    SaveConfig(instance->japiConfig);
    if(shouldSpawnConsole) {
        AllocConsole();
        SetConsoleTitle("JojoAPI Console");
        FILE* file = nullptr;
        freopen_s(&file, "CONOUT$", "w", stdout);
        freopen_s(&file, "CONIN$", "r", stdin);
    }

    if(MH_Initialize() != MH_OK) {
        JERROR("Failed to initialize MinHook!");
        return;
    }

    MODULEINFO info;
    GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(0), &info, sizeof(MODULEINFO));
    instance->dwSize = info.SizeOfImage;

    ScriptManager::Init();

    JINFO("Initialized JojoAPI!");
    instance->LoadMods();
}

void JAPI::InitThread(HINSTANCE hInstDll) {
    // For now janky loop
    while(true) {
        ScriptManager::ExecuteScripts();
        Sleep(1000);
    }
}

size_t JAPI::GetASBRModuleSize() {
    return instance->dwSize;
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

    if(!std::filesystem::exists("japi\\cpks")) {
        std::filesystem::create_directory("japi\\cpks");
    }

    // Load CPK mods
    LoadCPKMods();

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
        // Get mod filename
        std::string name = p.path().filename().string();

        if(name[0] == '-') {
            continue;
        }

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