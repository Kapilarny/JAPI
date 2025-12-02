//
// Created by kapil on 2.12.2025.
//

#ifndef JAPI_PRELOAD_MEM_H
#define JAPI_PRELOAD_MEM_H

#include <cstdint>
#include <Windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <stdio.h>

#define GAME_SCAN(sig) (uint64_t) PatternScan(GetModuleHandle(nullptr), sig)

// I stole this code from UC, credits to whoever made it
inline std::uint8_t* MemScan(void* module, const int* data, int size) {
    auto dosHeader = (PIMAGE_DOS_HEADER)module;
    auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

    auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
    auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

    auto s = size;
    auto d = data;

    for(auto i = 0ul; i < sizeOfImage - s; ++i) {
        bool found = true;
        for(auto j = 0ul; j < s; ++j) {
            if(scanBytes[i + j] != d[j] && d[j] != -1) {
                found = false;
                break;
            }
        }
        if(found) {
            return &scanBytes[i];
        }
    }

    return nullptr;
}

/*
 * @brief Scan for a given byte pattern on a module
 *
 * @Param module    Base of the module to search
 * @Param signature IDA-style byte array pattern
 *
 * @Returns Address of the first occurence
 */
inline std::uint8_t* PatternScan(void* module, const char* signature)
{
    static auto pattern_to_byte = [](const char* pattern) {
        auto bytes = std::vector<int>{};
        auto start = const_cast<char*>(pattern);
        auto end = const_cast<char*>(pattern) + strlen(pattern);

        for(auto current = start; current < end; ++current) {
            if(*current == '?') {
                ++current;
                if(*current == '?')
                    ++current;
                bytes.push_back(-1);
            } else {
                bytes.push_back(strtoul(current, &current, 16));
            }
        }
        return bytes;
    };

    auto bytes = pattern_to_byte(signature);
    auto scanBytes = MemScan(module, bytes.data(), static_cast<int>(bytes.size()));
    return scanBytes;
}

#endif //JAPI_PRELOAD_MEM_H