#include "game_data.h"

#include <filesystem>

#include "logger.h"
#include "mem.h"
#include "exports/JojoAPI.h"

GameData::GameData() {
    // Check if the game is ASBR or Connections
    if(std::filesystem::exists("ASBR.exe")) {
        this->game_type = GameType::ASBR;

        // Try scanning for functions
        this->functions.CPKPreloadFunc = GAME_SCAN("48 89 5C 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 48 83 3D");
        this->functions.CPKLoadFunc = GAME_SCAN("48 83 EC ? 44 8B C2 48 8D 54 24 ? E8 ? ? ? ? 48 83 C4");

        if(!this->functions.CPKLoadFunc || !this->functions.CPKPreloadFunc) {
            JWARN("Couldn't find the game functions! Defaulting to hardcoded!");

            this->functions.CPKPreloadFunc = JAPI_GetASBRModuleBase() + 0x56C970;
            this->functions.CPKLoadFunc = JAPI_GetASBRModuleBase() + 0x662378;
        }
    } else if(std::filesystem::exists("NSUNSC.exe")) {
        this->game_type = GameType::CONNECTIONS;

        // TODO: Pattern scan for NSUNSC too

        this->functions.CPKPreloadFunc = JAPI_GetASBRModuleBase() + 0xA7EED0;
        this->functions.CPKLoadFunc = JAPI_GetASBRModuleBase() + 0x5F7850;
    }
}
