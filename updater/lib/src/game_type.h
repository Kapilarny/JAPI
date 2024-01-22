#pragma once

#include <stdint.h>
#include <string>

enum class GameType { NONE = -1, ASBR = 0, CONNECTIONS = 1 };

typedef struct GameData {
  GameData();

  GameType game_type;
  std::string game_file;
  std::string steam_appid;
} GameData;

GameData &GetGameData();
