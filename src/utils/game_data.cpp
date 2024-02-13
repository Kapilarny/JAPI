#include "game_data.h"

#include <filesystem>

GameData::GameData() {
    // Check if the game is ASBR or Connections
    if(std::filesystem::exists("ASBR.exe")) {
        this->game_type = GameType::ASBR;
        
        this->functions.CPKPreloadFunc = 0x56C970;
        this->functions.CPKLoadFunc = 0x662378;
    } else if(std::filesystem::exists("NSUNSC.exe")) {
        this->game_type = GameType::CONNECTIONS;

        this->functions.CPKPreloadFunc = 0xA7EED0;
        this->functions.CPKLoadFunc = 0x5F7850;
    }
}