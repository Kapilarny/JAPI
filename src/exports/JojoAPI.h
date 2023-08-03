#pragma once

#include <Windows.h>
#include <string>

#define JEXP extern "C" __declspec(dllexport)

typedef struct ModMeta {
    const char* name;
    const char* guid;
    const char* version;
} ModMeta;

typedef struct Hook {
    void* target;
    void* detour;
    void* original;
    std::string name;
} Hook;

// Memory patching
JEXP void JAPI_PatchASBRMem(void* address, void* data, size_t size);
JEXP void JAPI_PatchMem(void* address, void* data, size_t size, HANDLE processHandle);

// Hooking
JEXP bool JAPI_HookFunction(Hook* hook);
JEXP bool JAPI_UnhookFunction(Hook* hook);
JEXP bool JAPI_HookASBRFunction(Hook* hook);
JEXP bool JAPI_UnhookASBRFunction(Hook* hook);

// Logging
JEXP void JAPI_LogFatal(std::string message);
JEXP void JAPI_LogError(std::string message);
JEXP void JAPI_LogWarn(std::string message);
JEXP void JAPI_LogInfo(std::string message);
JEXP void JAPI_LogDebug(std::string message);
JEXP void JAPI_LogTrace(std::string message);