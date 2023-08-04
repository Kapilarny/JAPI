#pragma once

#include <windows.h>

#include <sol.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>

typedef struct ScriptData {
    std::filesystem::file_time_type scriptTime;
    HANDLE threadHandle;
} ScriptData;

class ScriptManager {
public:
    static void Init();

    static void ExecuteScripts();

    static void AddFileToWatch(std::string scriptFilePath);
    static void RemoveFileFromWatch(std::string scriptFilePath);
private:
    static inline std::unique_ptr<ScriptManager> instance;

    void LoadCommands(sol::state& lua);

    std::unordered_map<std::string, ScriptData> watchedFiles;
};