#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "utils.h"

std::vector<uint8_t> DownloadFile(std::string url);

bool Is404(std::vector<uint8_t> &buffer);

Version GetLatestJAPIVersion();
std::vector<std::string> GetLatestJAPIDlls();

uint16_t DownloadASBR();
void DownloadJAPI(Version version);
void DownloadAdditionalDLLs();
void CreateSteamAppID();