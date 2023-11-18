#pragma once

#include <vector>
#include <string>
#include <stdint.h>

#include "utils.h"

std::vector<uint8_t> DownloadFile(std::string url);

bool Is404(std::vector<uint8_t>& buffer);

Version GetLatestJAPIVersion();
std::vector<std::string> GetLatestJAPIDlls();

uint16_t DownloadASBR();
void DownloadJAPI(Version version);
void DownloadAdditionalDLLs();
void RemoveEAC();