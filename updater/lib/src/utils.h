#pragma once

#include <vector>
#include <iostream>
#include <fstream>

#include "main.h"
#include "cppcrc.h"

void LaunchGame();
void UninstallEAC();

inline uint16_t ComputeCRC16Hash(std::ifstream& file) {
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    return CRC16::CCITT_FALSE::calc(buffer.data(), buffer.size());
}