//
// Created by user on 27.12.2024.
//

#include "JoJoAPI.h"

#include "japi.h"
#include "events/event.h"
#include "mods/mod_manager.h"
#include "utils/hooks.h"
#include "utils/logger.h"
#include "utils/mem.h"

dll_mod_meta* get_mod_meta(const void* ret_addr) {
    HMODULE hModule = nullptr;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)ret_addr, &hModule);

    return mod_manager::get_instance()->get_mod_meta(hModule);
}

const char* get_mod_guid(const void* ret_addr) {
    const dll_mod_meta* meta = get_mod_meta(ret_addr);
    if (meta == nullptr) {
        return "Unknown";
    }

    return meta->name;
}

uint64_t JAPI_GetModuleBase() {
    return japi::get_instance().get_module_base();
}

HMODULE JAPI_GetPluginModuleHandle(const char *pluginGUID) {
    return mod_manager::get_instance()->get_mod_hmodule(pluginGUID);
}

uint64_t JAPI_FindSignature(const char *signature) {
    return GAME_SCAN(signature);
}

char* JAPI_GetPluginReservedDir() {
    char* reserved_dir = new char[256];
    sprintf(reserved_dir, "japi\\dll-plugins\\%s", get_mod_guid(__builtin_return_address(0)));
    return reserved_dir;
}

void JAPI_RegisterEventCallback(const char *name, EventCallback callback) {
    event_transmitter::register_callback(name, callback);
}

void JAPI_TransmitEvent(const char *name, void *data) {
    event_transmitter::transmit_event(name, data);
}

bool JAPI_TransmitEventCancellable(const char *name, void *data) {
    return event_transmitter::transmit_event_cancellable(name, data);
}


bool JAPI_HookFunction(JAPIHook hook) {
    // TODO: register to the list of hooks

    return hooks::hook_function(hook);
}

bool JAPI_UnhookFunction(JAPIHook hook) {
    // TODO: unregister from the list of hooks

    return hooks::unhook_function(hook);
}

bool JAPI_HookGameFunction(JAPIHook hook) {
    hook.target += japi::get_instance().get_module_base();

    return JAPI_HookFunction(hook);
}

bool JAPI_UnhookGameFunction(JAPIHook hook) {
    hook.target += japi::get_instance().get_module_base();

    return JAPI_UnhookFunction(hook);
}

char* JAPI_ConfigBindString(const char *key, const char *defaultValue) {
    dll_mod_meta* meta = get_mod_meta(__builtin_return_address(0));
    if(!meta) {
        JFATAL("Tried to bind config key \"%s\" for unknown mod", key);
        return nullptr;
    }

    auto value = meta->mod_config.bind<std::string>(key, defaultValue);
    return strdup(value.c_str());
}

int JAPI_ConfigBindInt(const char *key, int defaultValue) {
    dll_mod_meta* meta = get_mod_meta(__builtin_return_address(0));
    if(!meta) {
        JFATAL("Tried to bind config key \"%s\" for unknown mod", key);
        return defaultValue;
    }

    return meta->mod_config.bind<int>(key, defaultValue);
}

float JAPI_ConfigBindFloat(const char *key, float defaultValue) {
    dll_mod_meta* meta = get_mod_meta(__builtin_return_address(0));
    if(!meta) {
        JFATAL("Tried to bind config key \"%s\" for unknown mod", key);
        return defaultValue;
    }

    return meta->mod_config.bind<float>(key, defaultValue);
}

bool JAPI_ConfigBindBool(const char *key, bool defaultValue) {
    dll_mod_meta* meta = get_mod_meta(__builtin_return_address(0));
    if(!meta) {
        JFATAL("Tried to bind config key \"%s\" for unknown mod", key);
        return defaultValue;
    }

    return meta->mod_config.bind<bool>(key, defaultValue);
}

void JAPI_LogFatal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    LogOutputVaList(LOG_LEVEL_FATAL, fmt, get_mod_guid(__builtin_extract_return_addr(__builtin_return_address(0))), args);

    va_end(args);
}

void JAPI_LogError(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    LogOutputVaList(LOG_LEVEL_ERROR, fmt, get_mod_guid(__builtin_extract_return_addr(__builtin_return_address(0))), args);

    va_end(args);
}

void JAPI_LogWarn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    LogOutputVaList(LOG_LEVEL_WARN, fmt, get_mod_guid(__builtin_extract_return_addr(__builtin_return_address(0))), args);

    va_end(args);
}

void JAPI_LogInfo(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    LogOutputVaList(LOG_LEVEL_INFO, fmt, get_mod_guid(__builtin_extract_return_addr(__builtin_return_address(0))), args);

    va_end(args);
}

void JAPI_LogDebug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    LogOutputVaList(LOG_LEVEL_DEBUG, fmt, get_mod_guid(__builtin_extract_return_addr(__builtin_return_address(0))), args);

    va_end(args);
}

void JAPI_LogTrace(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    LogOutputVaList(LOG_LEVEL_TRACE, fmt, get_mod_guid(__builtin_extract_return_addr(__builtin_return_address(0))), args);

    va_end(args);
}
