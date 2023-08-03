#pragma once

#include <Windows.h>
#include <stdio.h>

inline void PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess)
{
	DWORD oldprotect;
	VirtualProtectEx(hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	if (!WriteProcessMemory(hProcess, dst, src, size, nullptr)) {
		printf("[JojoAPI]: Failed to write to memory!\n");

		// Print the error code.
		printf("[JojoAPI]: Error code: %d\n", GetLastError());

		return;
	}
	VirtualProtectEx(hProcess, dst, size, oldprotect, &oldprotect);
}