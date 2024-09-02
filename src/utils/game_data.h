#pragma once

#include <stdint.h>

enum class GameType {
    ASBR = 0,
    CONNECTIONS = 1
};

typedef struct GameFunctions {
    uint64_t CPKPreloadFunc;
    uint64_t CPKLoadFunc;
} GameFunctions;

typedef struct GameData {
    GameData();

    GameType game_type;
    GameFunctions functions{};
} GameData;