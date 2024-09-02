#pragma once

#include <Windows.h>
#include <string>

#include "exports/JojoAPI.h"

typedef struct RegisteredConfigData {
    std::vector<std::pair<std::string, int*>> ints;
    std::vector<std::pair<std::string, bool*>> bools;
    std::vector<std::pair<std::string, char*>> strings;
    std::vector<std::pair<std::string, float*>> floats;
} RegisteredConfigData;

typedef struct ModData {
    ModMeta meta;
    HINSTANCE handle;
} ModData;