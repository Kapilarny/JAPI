#include "game_type.h"

#include <filesystem>

GameData::GameData() {
    if(std::filesystem::exists("ASBR.exe")) {
        this->game_type = GameType::ASBR;
        this->game_file = "ASBR.exe";
        this->steam_appid = "1372110";
    } else if(std::filesystem::exists("NSUNSC.exe")) {
        this->game_type = GameType::CONNECTIONS;
        this->game_file = "NSUNSC.exe";
        this->steam_appid = "1020790";
    }
}