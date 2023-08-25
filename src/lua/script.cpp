#include "script.h"

#include <MinHook.h>

#include "asm.h"
#include "utils/logger.h"
#include "utils/config.h"
#include "exports/JojoAPI.h"

void ScriptManager::Init() {
    InitASMExecutor();

    instance = std::make_unique<ScriptManager>();
}

void ScriptManager::ExecuteScripts() {
    // Create a new thread for each script
    for (auto& [scriptFilePath, scriptData] : instance->watchedFiles) {
        // Get the last modified time of the file
        auto lastModifiedTime = std::filesystem::last_write_time(scriptFilePath);

        // If the file has been modified since the last time we executed it, execute it again
        if(scriptData.threadHandle == nullptr || scriptData.scriptTime != lastModifiedTime) {
            scriptData.scriptTime = lastModifiedTime;

            // If the thread is still running, terminate it
            if (scriptData.threadHandle != nullptr) {
                if(scriptData.luaHandle["onUnload"].valid()) {
                    scriptData.luaHandle["onUnload"]();
                }

                TerminateThread(scriptData.threadHandle, 0);
                scriptData.luaHandle = sol::state();
            }

            // Create a new thread
            scriptData.threadHandle = CreateThread(nullptr, 0, [](LPVOID lpParam) -> DWORD {
                ScriptData* scriptData = (ScriptData*)lpParam;
                std::string scriptFilePath = scriptData->scriptFilePath;

                // Create a new Lua state
                scriptData->luaHandle.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package, sol::lib::os, sol::lib::io, sol::lib::coroutine, sol::lib::bit32, sol::lib::utf8, sol::lib::ffi, sol::lib::jit);

                instance->LoadCommands(scriptData->luaHandle, scriptData);

                // Execute the script
                scriptData->luaHandle.script_file(scriptFilePath);

                // Verify that the script has the config table
                if (!scriptData->luaHandle["config"].valid()) {
                    JERROR("Script does not have a config table: " + scriptFilePath);
                    return 0;
                }

                if (!scriptData->luaHandle["config"]["name"].is<std::string>()) {
                    JERROR("Invalid script metadata: " + scriptFilePath);
                    return 0;
                }

                if (!scriptData->luaHandle["config"]["guid"].is<std::string>()) {
                    JERROR("Invalid script metadata: " + scriptFilePath);
                    return 0;
                }

                if (!scriptData->luaHandle["config"]["version"].is<std::string>()) {
                    JERROR("Invalid script metadata: " + scriptFilePath);
                    return 0;
                }

                if (!scriptData->luaHandle["config"]["author"].is<std::string>()) {
                    JERROR("Invalid script metadata: " + scriptFilePath);
                    return 0;
                }

                // Verify that the script has an onLoad function
                if (!scriptData->luaHandle["onLoad"].valid()) {
                    JERROR("Invalid script metadata: " + scriptFilePath);
                    return 0;
                }

                scriptData->luaHandle["onLoad"]();

                return 0;
            }, (LPVOID) &scriptData, 0, nullptr);
        }
    }
}

void ScriptManager::LoadCommands(sol::state& lua, ScriptData* scriptData) {
    lua["log_trace"] = [&](std::string message) {
        LOG_TRACE(lua["config"]["name"], message);
    };

    lua["log_debug"] = [&](std::string message) {
        LOG_DEBUG(lua["config"]["name"], message);
    };

    lua["log_info"] = [&](std::string message) {
        LOG_INFO(lua["config"]["name"], message);
    };

    lua["log_warn"] = [&](std::string message) {
        LOG_WARN(lua["config"]["name"], message);
    };

    lua["log_error"] = [&](std::string message) {
        LOG_ERROR(lua["config"]["name"], message);
    };

    lua["log_fatal"] = [&](std::string message) {
        LOG_FATAL(lua["config"]["name"], message);
    };

    lua["patch_mem"] = [&](uintptr_t address, std::string data) {
        JAPI_PatchMem((void*)address, data.data(), data.size());
    };

    lua["patch_asbr_mem"] = [&](uintptr_t address, std::string data) {
        JAPI_PatchASBRMem((void*)address, data.data(), data.size());
    };

    lua["copy_mem"] = [&](uintptr_t src, size_t size) {
        std::string dataStr(size, 0);
        JAPI_CopyMem((void*)dataStr.data(), (void*)src, size);

        return dataStr;
    };

    lua["copy_asbr_mem"] = [&](uintptr_t src, size_t size) {
        std::string dataStr(size, 0);
        JAPI_CopyASBRMem((void*)dataStr.data(), (void*)src, size);

        return dataStr;
    };

    lua["config_bind_string"] = [&](std::string key, std::string defaultVal) {
        auto config = GetModConfig(lua["config"]["name"]);
        auto value = ConfigBind(config.table, key, defaultVal);

        SaveConfig(config);

        return value;
    };

    lua["config_bind_bool"] = [&](std::string key, bool defaultVal) {
        auto config = GetModConfig(lua["config"]["name"]);
        auto value = ConfigBind(config.table, key, defaultVal);

        SaveConfig(config);

        return value;
    };

    lua["config_bind_int"] = [&](std::string key, int defaultVal) {
        auto config = GetModConfig(lua["config"]["name"]);
        auto value = ConfigBind(config.table, key, defaultVal);

        SaveConfig(config);

        return value;
    };

    lua["config_bind_float"] = [&](std::string key, float defaultVal) {
        auto config = GetModConfig(lua["config"]["name"]);
        auto value = ConfigBind(config.table, key, defaultVal);

        SaveConfig(config);

        return value;
    };

    lua["get_proc_address"] = [&](std::string module, std::string proc) {
        return (uintptr_t)GetProcAddress(GetModuleHandleA(module.c_str()), proc.c_str());
    };

    lua["mem_alloc"] = [&](size_t size) {
        return (uintptr_t)malloc(size);
    };

    lua["mem_free"] = [&](uintptr_t address) {
        free((void*)address);
    };

    lua["mem_write"] = [&](uintptr_t address, std::string data, size_t size) {
        memcpy((void*)address, (void*)data.data(), size);
    };

    lua["exec_asm"] = [&](std::string code) {
        ExecuteASMCode(code);
    };

    lua["hook_func"] = [&](uintptr_t target, sol::protected_function callback) {
        callback(); // This is fine

        scriptData->bpHooks.push_back(std::make_unique<BreakpointHook>(target, [&](PEXCEPTION_POINTERS info) -> void {
            JINFO("Called motherfucker"); // This is fine
            callback(); 
        }));
    };
}

void ScriptManager::AddFileToWatch(std::string scriptFilePath) {
    if (!std::filesystem::exists(scriptFilePath)) {
        JERROR("Script file does not exist: " + scriptFilePath);
        return;
    }

    instance->watchedFiles[scriptFilePath] = {
        std::filesystem::last_write_time(scriptFilePath),
        scriptFilePath,
        nullptr,
        sol::state()
    };
}

void ScriptManager::RemoveFileFromWatch(std::string scriptFilePath) {
    instance->watchedFiles.erase(scriptFilePath);
}