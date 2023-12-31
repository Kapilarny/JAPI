#pragma once

#include <windows.h>

#include <sol.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include "bp_hook.h"

typedef struct ScriptData {
    std::filesystem::file_time_type scriptTime;
    std::string scriptFilePath;
    HANDLE threadHandle;
    sol::state luaHandle;
    std::vector<std::unique_ptr<BreakpointHook>> bpHooks;
} ScriptData;

class ScriptManager {
public:
    static void Init();

    static void ExecuteScripts();
    static sol::state& GetLuaState(std::string scriptFilePath);

    static void AddFileToWatch(std::string scriptFilePath);
    static void RemoveFileFromWatch(std::string scriptFilePath);
private:
    static inline std::unique_ptr<ScriptManager> instance;

    void LoadCommands(sol::state& lua, ScriptData* scriptData);

    std::unordered_map<std::string, ScriptData> watchedFiles;
};