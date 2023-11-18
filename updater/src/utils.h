#pragma once

#include <vector>
#include <iostream>
#include <fstream>

#include "cppcrc.h"

typedef struct Version {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} Version;

void LaunchGame();
void UninstallEAC();

inline uint16_t ComputeCRC16Hash(std::ifstream& file) {
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    return CRC16::CCITT_FALSE::calc(buffer.data(), buffer.size());
}

inline std::string VersionString(Version version) {
    return std::to_string(version.major) + "." + std::to_string(version.minor) + "." + std::to_string(version.patch);
}

inline bool IsBiggerVersion(Version a, Version b) {
    if(a.major > b.major) {
        return true;
    }

    if(a.major < b.major) {
        return false;
    }

    if(a.minor > b.minor) {
        return true;
    }

    if(a.minor < b.minor) {
        return false;
    }

    if(a.patch > b.patch) {
        return true;
    }

    if(a.patch < b.patch) {
        return false;
    }

    return false;
}

inline Version ParseVersion(std::string version) {
    Version v;
    std::string::iterator it = version.begin();
    std::string::iterator end = version.end();

    std::string major;
    std::string minor;
    std::string patch;

    for(; it != end; it++) {
        if(*it == '.') {
            break;
        }

        major += *it;
    }

    it++;

    for(; it != end; it++) {
        if(*it == '.') {
            break;
        }

        minor += *it;
    }

    it++;

    for(; it != end; it++) {
        if(*it == '.') {
            break;
        }

        patch += *it;
    }

    v.major = std::stoi(major);
    v.minor = std::stoi(minor);
    v.patch = std::stoi(patch);

    return v;
}