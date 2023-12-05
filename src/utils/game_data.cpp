#include "game_data.h"

#include <filesystem>

GameData::GameData() {
    // Check if the game is ASBR or Connections
    if(std::filesystem::exists("ASBR.exe")) {
        this->game_type = GameType::ASBR;
        
        this->functions.CPKPreloadFunc = 0x56C5D0;
        this->functions.CPKLoadFunc = 0x661FD8;
    } else if(std::filesystem::exists("NSUNSC.exe")) {
        this->game_type = GameType::CONNECTIONS;

        this->functions.CPKPreloadFunc = 0xa7eed0;
        this->functions.CPKLoadFunc = 0xa7ef58;
    }
}