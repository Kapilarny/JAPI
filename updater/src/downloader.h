#pragma once

#include <vector>
#include <string>
#include <stdint.h>

#include "utils.h"

std::vector<uint8_t> DownloadFile(std::string url);

Version GetCurrentJAPIVersion();