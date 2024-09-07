#include "JojoAPI.h"

#include <windows.h>
#include <tlhelp32.h>

#include <utility>

#include "japi.h"

#include "utils/config.h"
#include "utils/logger.h"
#include "utils/mem.h"
#include "utils/hooks.h"

#include "events/event.h"

// #include "lua/asm.h"

const char* GetModGUID(void* retAddr) {
    HMODULE hModule = NULL;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)retAddr, &hModule);

    // Get the mod info
    auto getModInfo = (ModMeta (*)()) GetProcAddress(hModule, "GetModInfo");
    if(!getModInfo) {
        return "Unknown";
    }
    
    return getModInfo().guid;
}

__int64 JAPI_GetASBRModuleBase() {
    return JAPI::GetASBRModuleBase();
}

__int64 JAPI_FindSignature(const char* signature) {
    return GAME_SCAN(signature);
}

void JAPI_RegisterEventCallback(std::string eventName, EventCallback callback) {
    EventTransmitter::RegisterCallback(eventName, callback);
}

void JAPI_PatchASBRMem(void* address, void* data, size_t size) {
    PatchEx((BYTE*) (JAPI::GetASBRModuleBase() + (uint64_t) address), (BYTE*)data, size);
}

void JAPI_PatchMem(void* address, void* data, size_t size) {
    PatchEx((BYTE*)address, (BYTE*)data, size);
}

void JAPI_CopyASBRMem(void* dest, void* src, size_t size) {
    memcpy((BYTE*) dest, (BYTE*) (JAPI::GetASBRModuleBase() + (uint64_t) src), size);
}

void JAPI_CopyMem(void* dest, void* src, size_t size) {
    memcpy((BYTE*)dest, (BYTE*)src, size);
}

std::string JAPI_GetPluginReservedDir() {
    return "japi\\dll-plugins\\" + std::string(GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))));
}

// void JAPI_ExecuteASMCode(std::string code) {
//     ExecuteASMCode(code);
// }

bool JAPI_HookFunction(Hook* hook) {
    return HookFunction(hook);
}

bool JAPI_UnhookFunction(Hook* hook) {
    return UnhookFunction(hook);
}

bool JAPI_HookASBRFunction(Hook* hook) {
    hook->target = (void*) (JAPI::GetASBRModuleBase() + (uint64_t) hook->target);
    return HookFunction(hook);
}

bool JAPI_UnhookASBRFunction(Hook* hook) {
    hook->target = (void*) (JAPI::GetASBRModuleBase() + (uint64_t) hook->target);
    return UnhookFunction(hook);
}

void JAPI_LogFatal(std::string message, ...) {
    va_list args;
    va_start(args, message);

    LogOutputVaList(LOG_LEVEL_FATAL, message, GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))), args);

    va_end(args);
}

void JAPI_LogError(std::string message, ...) {
    va_list args;
    va_start(args, message);

    LogOutputVaList(LOG_LEVEL_ERROR, message, GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))), args);

    va_end(args);
}

void JAPI_LogWarn(std::string message, ...) {
    va_list args;
    va_start(args, message);

    LogOutputVaList(LOG_LEVEL_WARN, message, GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))), args);

    va_end(args);
}

void JAPI_LogInfo(std::string message, ...) {
    va_list args;
    va_start(args, message);

    LogOutputVaList(LOG_LEVEL_INFO, message, GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))), args);

    va_end(args);
}

void JAPI_LogDebug(std::string message, ...) {
    va_list args;
    va_start(args, message);

    LogOutputVaList(LOG_LEVEL_DEBUG, message, GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))), args);

    va_end(args);
}

void JAPI_LogTrace(std::string message, ...) {
    va_list args;
    va_start(args, message);

    LogOutputVaList(LOG_LEVEL_TRACE, message, GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))), args);

    va_end(args);
}

std::string JAPI_ConfigBindString(std::string key, std::string defaultValue) {
    ModConfig config = GetModConfig(GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))));
    auto value = ConfigBind(config.table, key, defaultValue);
    
    // Save the config
    SaveConfig(config);

    return value;
}

int JAPI_ConfigBindInt(std::string key, int defaultValue) {
    ModConfig config = GetModConfig(GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))));
    auto value = ConfigBind(config.table, key, defaultValue);
    
    // Save the config
    SaveConfig(config);

    return value;
}

float JAPI_ConfigBindFloat(std::string key, float defaultValue) {
    ModConfig config = GetModConfig(GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))));
    auto value = ConfigBind(config.table, key, defaultValue);
    
    // Save the config
    SaveConfig(config);

    return value;
}

bool JAPI_ConfigBindBool(std::string key, bool defaultValue) {
    ModConfig config = GetModConfig(GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))));
    auto value = ConfigBind(config.table, key, defaultValue);
    
    // Save the config
    SaveConfig(config);

    return value;
}

bool JAPI_ConfigRegisterInt(int* value, std::string key, int defaultValue) {
    auto guid = GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0)));
    ModConfig config = GetModConfig(guid);

    ConfigRegister(config.table, value, key, defaultValue);

    // Save the config
    SaveConfig(config);

    JAPI::GetRegisteredDataMap()[guid].ints.emplace_back(key, value);

    return true;
}

bool JAPI_ConfigRegisterFloat(float* value, std::string key, float defaultValue) {
    auto guid = GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0)));
    ModConfig config = GetModConfig(guid);

    ConfigRegister(config.table, value, key, defaultValue);

    // Save the config
    SaveConfig(config);

    JAPI::GetRegisteredDataMap()[guid].floats.emplace_back(key, value);

    return true;
}

bool JAPI_ConfigRegisterBool(bool* value, std::string key, bool defaultValue) {
    auto guid = GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0)));
    ModConfig config = GetModConfig(guid);

    ConfigRegister(config.table, value, key, defaultValue);

    // Save the config
    SaveConfig(config);

    JAPI::GetRegisteredDataMap()[guid].bools.emplace_back(key, value);

    return true;
}

JEXP bool JAPI_ConfigRegisterString(char* buffer, std::string key, const char* defaultString) {
    auto guid = GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0)));
    ModConfig config = GetModConfig(guid);

    if(config.table.contains(key)) {
        std::string str = config.table[key].value_or(std::string(defaultString));
        strcpy(buffer, str.c_str());
    } else {
        strcpy(buffer, defaultString);
        config.table.insert_or_assign(key, std::string(defaultString));
    }

    // Save the config
    SaveConfig(config);

    JAPI::GetRegisteredDataMap()[guid].strings.emplace_back(key, buffer);

    return true;
}