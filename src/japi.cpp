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
// #include "lua/asm.h"

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

    // Load GameData
    instance->gameData = GameData();

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

    if(!std::filesystem::exists("japi\\dll-plugins")) {
        std::filesystem::create_directory("japi\\dll-plugins");
    }

    if(!std::filesystem::exists("japi\\lua-plugins")) {
        std::filesystem::create_directory("japi\\lua-plugins");
    }

    if(!std::filesystem::exists("japi\\cpks")) {
        std::filesystem::create_directory("japi\\cpks");
    }

    if(std::filesystem::exists("japi\\mods")) {
        for(auto& p : std::filesystem::directory_iterator("japi\\mods")) {
            if(p.path().extension() == ".dll") {
                LOG_WARN("PluginLoader", "Please move your plugins from japi\\mods to japi\\dll-plugins!");
                LOG_WARN("PluginLoader", "This error message will be removed in a future version!");
                break;
            }
        }
    }

    if(std::filesystem::exists("japi\\luamods")) {
        for(auto& p : std::filesystem::directory_iterator("japi\\luamods")) {
            if(p.path().extension() == ".lua") {
                LOG_WARN("PluginLoader", "Please move your plugins from japi\\luamods to japi\\lua-plugins!");
                LOG_WARN("PluginLoader", "This error message will be removed in a future version!");
                break;
            }
        }
    }

    // Load CPK mods
    LoadCPKMods();

    // Get all files in japi\mods and load them
    for(auto& p : std::filesystem::directory_iterator("japi\\dll-plugins")) {
        if(p.path().extension() == ".dll") {
            // Load the DLL
            auto handle = LoadLibrary(p.path().string().c_str());
            if(!handle) {
                LOG_ERROR("PluginLoader", "Failed to load plugin " + p.path().string());
                continue;
            }

            // Get the mod info
            auto getModInfo = (ModMeta (*)()) GetProcAddress(handle, "GetModInfo");
            if(!getModInfo) {
                LOG_ERROR("PluginLoader", "Failed to get plugin info for " + p.path().string());
                continue;
            }

            ModMeta modInfo = getModInfo();
            auto modInit = (void (*)()) GetProcAddress(handle, "ModInit");
            if(!modInit) {
                LOG_ERROR("PluginLoader", "Failed to get plugin init for " + p.path().string());
                continue;
            }

            modInit();

            mods.push_back({modInfo, handle});
        }
    }

    // Get all files in japi\luamods and load them
    for(auto& p : std::filesystem::directory_iterator("japi\\lua-plugins")) {
        // Get mod filename
        std::string name = p.path().filename().string();

        if(name[0] == '-') {
            continue;
        }

        if(p.path().extension() == ".lua") {
            LOG_INFO("PluginLoader", "Loading Lua plugin " + p.path().string());
            ScriptManager::AddFileToWatch(p.path().string());
        }
    }

    LOG_INFO("PluginLoader", "Loaded " + std::to_string(mods.size()) + " plugins!");
}

std::string JAPI::GetModGUID(HANDLE modHandle) {
    for(auto& mod : instance->mods) {
        if(mod.handle == modHandle) {
            return mod.meta.guid;
        }
    }

    return "";
}

GameData& JAPI::GetGameData() {
    return instance->gameData;
}