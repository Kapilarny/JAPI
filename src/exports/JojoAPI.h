#pragma once

#include <Windows.h>
#include <string>

#include "JAPIEvents.h"

#define JEXP extern "C" __declspec(dllexport)

typedef struct ModMeta {
    const char* name;
    const char* guid;
    const char* version;
    const char* author;
} ModMeta;

typedef struct Hook {
    void* target;
    void* detour;
    void* original;
    std::string name;
} Hook;

// Memory patching
JEXP __int64 JAPI_GetASBRModuleBase();

JEXP __int64 JAPI_FindSignature(const char* signature);
JEXP void JAPI_PatchASBRMem(void* address, void* data, size_t size);
JEXP void JAPI_PatchMem(void* address, void* data, size_t size);
JEXP void JAPI_CopyASBRMem(void* dest, void* src, size_t size);
JEXP void JAPI_CopyMem(void* dest, void* src, size_t size);
JEXP void JAPI_ExecuteASMCode(std::string code);

// Hooking
JEXP bool JAPI_HookFunction(Hook* hook);
JEXP bool JAPI_UnhookFunction(Hook* hook);
JEXP bool JAPI_HookASBRFunction(Hook* hook);
JEXP bool JAPI_UnhookASBRFunction(Hook* hook);

// Logging
JEXP void JAPI_LogFatal(std::string message, ...);
JEXP void JAPI_LogError(std::string message, ...);
JEXP void JAPI_LogWarn(std::string message, ...);
JEXP void JAPI_LogInfo(std::string message, ...);
JEXP void JAPI_LogDebug(std::string message, ...);
JEXP void JAPI_LogTrace(std::string message, ...);

// Config
JEXP std::string JAPI_ConfigBindString(std::string key, std::string defaultValue);
JEXP int JAPI_ConfigBindInt(std::string key, int defaultValue);
JEXP float JAPI_ConfigBindFloat(std::string key, float defaultValue);
JEXP bool JAPI_ConfigBindBool(std::string key, bool defaultValue);

// Live editable config
JEXP bool JAPI_ConfigRegisterInt(int* value, std::string key, int defaultValue);
JEXP bool JAPI_ConfigRegisterFloat(float* value, std::string key, float defaultValue);
JEXP bool JAPI_ConfigRegisterBool(bool* value, std::string key, bool defaultValue);
// WARNING: Buffer ***MUST*** be of size 255, and its creation/deletion is handled by user
JEXP bool JAPI_ConfigRegisterString(char* buffer, std::string key, const char* defaultString);

// Plugin Directory
JEXP std::string JAPI_GetPluginReservedDir();

// Events
JEXP void JAPI_RegisterEventCallback(std::string eventName, EventCallback callback);
