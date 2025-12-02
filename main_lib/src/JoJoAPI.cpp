//
// Created by kapil on 1.12.2025.
//

#include "JoJoAPI.h"

#include <filesystem>

#include "japi.h"
#include "subsystems/events.h"
#include "subsystems/hook.h"
#include "subsystems/logger.h"
#include "util/mem.h"

#define GET_MOD_META() get_mod_meta_inner(__builtin_return_address(0))

dll_mod_meta* get_mod_meta_inner(const void* ret_addr) {
    HMODULE hModule = nullptr;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)ret_addr, &hModule);

    return modloader::get().get_mod_meta(hModule);
}

uint64_t JAPI_GetModuleBaseAddress() {
    return (uint64_t)japi::get().get_module_base();
}

uint64_t JAPI_FindSignature(const char *signature) {
    return GAME_SCAN(signature);
}

uint64_t JAPI_FindString(const char *str) {
    auto strl = strlen(str);
    auto size_in_dwords = (strl / 4) + ((strl % 4) ? 1 : 0);
    return (uint64_t)MemScan(japi::get().get_module_base(), reinterpret_cast<const int*>(str), size_in_dwords);
}

JAPIHookHandle JAPI_RegisterHook(JAPIHookMeta hook_meta) {
    return hook_manager::get().register_hook(hook_meta);
}

bool JAPI_UnregisterHook(JAPIHookHandle hook_handle) {
    return hook_manager::get().unregister_hook(hook_handle);
}

JAPIString* JAPI_ConfigBindString(const char *key, const char *default_value) {
    const auto meta = GET_MOD_META();
    if (!meta) {
        JFATAL("Tried to bind config key \"%s\" for an unknown mod", key);
        return nullptr;
    }

    const auto val = meta->mod_config.bind<std::string>(key, default_value);
    const auto copy = strdup(val.c_str());

    const auto result = new JAPIString(copy, val.size());
    return result;
}

int JAPI_ConfigBindInt(const char *key, int default_value) {
    const auto meta = GET_MOD_META();
    if (!meta) {
        JFATAL("Tried to bind config key \"%s\" for an unknown mod", key);
        return default_value;
    }

    return meta->mod_config.bind(key, default_value);
}

bool JAPI_ConfigBindBool(const char *key, bool default_value) {
    const auto meta = GET_MOD_META();
    if (!meta) {
        JFATAL("Tried to bind config key \"%s\" for an unknown mod", key);
        return default_value;
    }

    return meta->mod_config.bind(key, default_value);
}

float JAPI_ConfigBindFloat(const char *key, float default_value) {
    const auto meta = GET_MOD_META();
    if (!meta) {
        JFATAL("Tried to bind config key \"%s\" for an unknown mod", key);
        return default_value;
    }

    return meta->mod_config.bind<int>(key, default_value);
}

void JAPI_ConfigSetString(const char *key, const char *value) {
    const auto meta = GET_MOD_META();
    if (!meta) {
        JFATAL("Tried to set config key \"%s\" for an unknown mod", key);
        return;
    }

    meta->mod_config.set(key, value);
}

void JAPI_ConfigSetInt(const char *key, int value) {
    const auto meta = GET_MOD_META();
    if (!meta) {
        JFATAL("Tried to set config key \"%s\" for an unknown mod", key);
        return;
    }

    meta->mod_config.set(key, value);
}

void JAPI_ConfigSetBool(const char *key, bool value) {
    const auto meta = GET_MOD_META();
    if (!meta) {
        JFATAL("Tried to set config key \"%s\" for an unknown mod", key);
        return;
    }

    meta->mod_config.set(key, value);
}

void JAPI_ConfigSetFloat(const char *key, float value) {
    const auto meta = GET_MOD_META();
    if (!meta) {
        JFATAL("Tried to set config key \"%s\" for an unknown mod", key);
        return;
    }

    meta->mod_config.set(key, value);
}

void JAPI_RegisterEventListener(const char *event_name, JAPI_EventListener listener) {
    event_manager::get().register_event_listener(event_name, listener);
}

void JAPI_RegisterCancellableEventListener(const char *event_name, JAPI_CancellableEventListener listener) {
    event_manager::get().register_cancellable_event_listener(event_name, listener);
}

void JAPI_TransmitEvent(const char *event_name, void *event_data) {
    event_manager::get().transmit_event(event_name, event_data);
}

void JAPI_TransmitEventCancellable(const char *event_name, void *event_data, bool *cancelled) {
    *cancelled = event_manager::get().transmit_event_cancellable(event_name, event_data);
}

JAPIString* JAPI_GetPluginReservedDirectory() {
    const auto meta = GET_MOD_META();
    if (!meta) {
        JFATAL("Tried to get reserved directory for an unknown mod");
        return nullptr;
    }
    std::string path = "japi/plugins/" + std::string(meta->guid) + "/";
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }

    // Copy the string to a new memory location
    const auto copy = strdup(path.c_str());
    const auto result = new JAPIString(copy, path.size());
    return result;
}

void JAPI_FreeString(JAPIString *japi_string) {
    if (!japi_string) return;
    free((char*)japi_string->data);
    delete japi_string;
}

void JAPI_LogMessage(JAPI_LOG_LEVEL level, const char *message, ...) {
    va_list args;
    va_start(args, message);
    log_output_va_list((LogLevel)level, message, GET_MOD_META() ? GET_MOD_META()->guid : "Unknown", args);
    va_end(args);
}