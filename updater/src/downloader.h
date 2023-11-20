#pragma once

#include <vector>
#include <string>
#include <stdint.h>

#include "utils.h"

bool Is404(std::vector<uint8_t>& buffer);
std::vector<uint8_t> DownloadFile(std::string url);
Version GetLatestUpdaterVersion();
void DownloadUpdater(Version version);