#pragma once

#include <Windows.h>
#include <stdio.h>

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

inline void* FindSignature(char* begin, size_t size, const char* pattern, const char* mask) {
	unsigned int patternLength = strlen(pattern);

	for (unsigned int i = 0; i < size - patternLength; i++) {
		bool found = true;
		for (unsigned int j = 0; j < patternLength; j++) {
			if (mask[j] != '?' && pattern[j] !=*(begin + i + j)) {
				found = false;
				break;
			}
		}
		if (found) {
			return (begin + i);
		}
	}
	return nullptr;
}