//
// Created by user on 27.12.2024.
//

#ifndef JOJOAPI_H
#define JOJOAPI_H

#define JEXP extern "C" __declspec(dllexport)

#include <cstdint>
#include <cstdbool>

#include "JAPIEvents.h"

typedef struct JAPIModMeta {
    const char* name;
    const char* guid;
    const char* author;
    const char* version;
    const char* description;
} JAPIModMeta;

typedef struct JAPIHook {
    uint64_t target;
    void* detour;
    void* original;
    const char* name;
} JAPIHook;

typedef struct JAPIModDependencies {
    size_t dependencies_count;
    const char** dependencies;
} JAPIModDependencies;

// Memory
JEXP uint64_t JAPI_GetModuleBase();
JEXP uint64_t JAPI_FindSignature(const char* signature);

// Hooking
JEXP bool JAPI_HookFunction(JAPIHook hook);
JEXP bool JAPI_UnhookFunction(JAPIHook hook);
JEXP bool JAPI_HookGameFunction(JAPIHook hook);
JEXP bool JAPI_UnhookGameFunction(JAPIHook hook);

// Logging
JEXP void JAPI_LogFatal(const char* fmt, ...);
JEXP void JAPI_LogError(const char* fmt, ...);
JEXP void JAPI_LogWarn(const char* fmt, ...);
JEXP void JAPI_LogInfo(const char* fmt, ...);
JEXP void JAPI_LogDebug(const char* fmt, ...);
JEXP void JAPI_LogTrace(const char* fmt, ...);

// Config
JEXP char* JAPI_ConfigBindString(const char* key, const char* defaultValue); // WARNING: The callee is responsible for freeing the returned string
JEXP int JAPI_ConfigBindInt(const char* key, int defaultValue);
JEXP float JAPI_ConfigBindFloat(const char* key, float defaultValue);
JEXP bool JAPI_ConfigBindBool(const char* key, bool defaultValue);

// Misc
JEXP char* JAPI_GetPluginReservedDir(); // WARNING: The callee is responsible for freeing the returned string

// Events
JEXP void JAPI_RegisterEventCallback(const char* name, EventCallback callback);
JEXP void JAPI_TransmitEvent(const char* name, void* data); // Custom events :)
JEXP bool JAPI_TransmitEventCancellable(const char* name, void* data); // Custom events :)

#endif //JOJOAPI_H
