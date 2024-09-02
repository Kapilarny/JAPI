#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "mod.h"
#include "utils/config.h"
#include "utils/game_data.h"

class JAPI {
public:
    static void Init(HINSTANCE hinstDLL);
    static void InitThread(HINSTANCE hinstDLL);

    static uint64_t GetASBRModuleBase();

    static size_t GetASBRModuleSize();
    static std::string GetModGUID(HANDLE modHandle);
    static GameData& GetGameData();

    static std::string GetJAPIVersionString() { return "3.0.1"; }
    static const std::vector<ModData>& GetMods() { return instance->mods; }
    static std::unordered_map<std::string, RegisteredConfigData>& GetRegisteredDataMap() { return instance->mod_registered_data; }
    static RegisteredConfigData GetConfigData(const std::string& guid) { return instance->mod_registered_data[guid]; }
private:
    JAPI() = default;

    void LoadMods();
    void LoadDllPlugins();
    void LoadLuaPlugins();

    GameData gameData;

    uint64_t asbrModuleBase{};
    HINSTANCE hinstDLL{};
    size_t dwSize{};

    ModConfig japiConfig;
    ModConfig pluginLoaderConfig;
    
    std::vector<ModData> mods;
    std::unordered_map<std::string, RegisteredConfigData> mod_registered_data;

    static inline std::unique_ptr<JAPI> instance;    
};