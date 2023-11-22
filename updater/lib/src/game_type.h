#pragma once

#include <string>
#include <stdint.h>

enum class GameType {
    NONE = -1,
    ASBR = 0,
    CONNECTIONS = 1
};

typedef struct GameData {
    GameData();

    GameType game_type;
    std::string game_file;
    std::string steam_appid;
} GameData;

GameData& GetGameData();