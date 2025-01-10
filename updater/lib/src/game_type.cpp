#include "game_type.h"

#include <filesystem>

#include "logger.h"

GameData::GameData() {
    if(std::filesystem::exists("ASBR.exe")) {
        JINFO("Detected ASBR.exe, assuming ASBR");

        this->game_type = GameType::ASBR;
        this->game_file = "ASBR.exe.unpacked.exe";
        this->steam_appid = "1372110";
    } else if(std::filesystem::exists("NSUNSC.exe")) {
        JINFO("Detected NSUNSC.exe, assuming Connections");

        this->game_type = GameType::CONNECTIONS;
        this->game_file = "NSUNSC.exe";
        this->steam_appid = "1020790";
    } else {
        JFATAL("Could not detect game type");
        exit(1);
    }
}

static GameData game_data;

GameData& GetGameData() {
    if(game_data.game_type == GameType::NONE) {
        game_data = GameData();
    }

    return game_data;
}