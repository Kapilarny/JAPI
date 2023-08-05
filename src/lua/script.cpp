#include "script.h"

#include "utils/logger.h"
#include "utils/config.h"
#include "exports/JojoAPI.h"
#include <MinHook.h>

void ScriptManager::Init() {
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
                TerminateThread(scriptData.threadHandle, 0);
            }

            // Create a new thread
            scriptData.threadHandle = CreateThread(nullptr, 0, [](LPVOID lpParam) -> DWORD {
                auto scriptFilePath = (std::string*)lpParam;

                // Create a new Lua state
                sol::state lua;
                lua.open_libraries(sol::lib::base);

                instance->LoadCommands(lua);

                // Execute the script
                lua.script_file(*scriptFilePath);

                // Verify that the script has the config table
                if (!lua["config"].valid()) {
                    JERROR("Script does not have a config table: " + *scriptFilePath);
                    return 0;
                }

                if (!lua["config"]["name"].is<std::string>()) {
                    JERROR("Invalid script metadata: " + *scriptFilePath);
                    return 0;
                }

                if (!lua["config"]["guid"].is<std::string>()) {
                    JERROR("Invalid script metadata: " + *scriptFilePath);
                    return 0;
                }

                if (!lua["config"]["version"].is<std::string>()) {
                    JERROR("Invalid script metadata: " + *scriptFilePath);
                    return 0;
                }

                if (!lua["config"]["author"].is<std::string>()) {
                    JERROR("Invalid script metadata: " + *scriptFilePath);
                    return 0;
                }

                // Verify that the script has an onLoad function
                if (!lua["onLoad"].valid()) {
                    JERROR("Invalid script metadata: " + *scriptFilePath);
                    return 0;
                }

                lua["onLoad"]();

                return 0;
            }, (LPVOID) &scriptFilePath, 0, nullptr);
        }
    }
}

void ScriptManager::LoadCommands(sol::state& lua) {
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
}

void ScriptManager::AddFileToWatch(std::string scriptFilePath) {
    if (!std::filesystem::exists(scriptFilePath)) {
        JERROR("Script file does not exist: " + scriptFilePath);
        return;
    }

    instance->watchedFiles[scriptFilePath] = {};
}

void ScriptManager::RemoveFileFromWatch(std::string scriptFilePath) {
    instance->watchedFiles.erase(scriptFilePath);
}