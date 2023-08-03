#pragma once

#include <Windows.h>
#include <stdio.h>

inline void PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess)
{
	DWORD oldprotect;
	VirtualProtectEx(hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	if (!WriteProcessMemory(hProcess, dst, src, size, nullptr)) {
		JERROR("Failed to write to process memory.!");

		// Print the error code.
		DWORD errorCode = GetLastError();
		JERROR("Error code: " + std::to_string(errorCode));

		return;
	}
	VirtualProtectEx(hProcess, dst, size, oldprotect, &oldprotect);
}