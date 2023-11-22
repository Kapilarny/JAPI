#pragma once

#include <memory>
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
private:
    JAPI() = default;

    void LoadMods();

    GameData gameData;

    uint64_t asbrModuleBase;
    HINSTANCE hinstDLL;
    size_t dwSize;

    ModConfig japiConfig;
    
    std::vector<ModData> mods;

    static inline std::unique_ptr<JAPI> instance;    
};