#pragma once

#include <Windows.h>
#include <string>

#include "exports/JojoAPI.h"

typedef struct ModData {
    ModMeta meta;
    HINSTANCE handle;
} ModData;