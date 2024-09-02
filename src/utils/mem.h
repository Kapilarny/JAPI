#pragma once

#include <Windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <stdio.h>

#define GAME_SCAN(sig) (uint64_t) PatternScan(GetModuleHandle(nullptr), sig)

inline void PatchEx(BYTE* dst, BYTE* src, unsigned int size) {
	DWORD oldprotect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	memcpy(dst, src, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}

inline bool MemoryCompare(const BYTE* data, const BYTE* mask, const char* szMask) {
	for (; *szMask; ++szMask, ++data, ++mask) {
		if (*szMask == 'x' && *data != *mask) {
			return false;
		}
	}

	return (*szMask == 0);
}

// I stole this code from UC, credits to whoever made it
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

    auto dosHeader = (PIMAGE_DOS_HEADER)module;
    auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

    auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
    auto patternBytes = pattern_to_byte(signature);
    auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

    auto s = patternBytes.size();
    auto d = patternBytes.data();

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
