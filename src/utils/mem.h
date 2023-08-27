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

inline DWORD FindSignature(__int64 sigStart, size_t sigSize, const char* signature, const char* mask) {
	byte* data = new byte[sigSize];
	size_t bytesRead;

	memcpy(data, (void*)sigStart, sigSize);
	for(DWORD i = 0; i < sigSize; i++) {
		if(MemoryCompare((const BYTE*)(data + i), (const BYTE*)signature, mask)) {
			delete[] data;
			return sigStart + i;
		}
	}

	delete[] data;
	return 0;
}