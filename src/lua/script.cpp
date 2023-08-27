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

typedef struct BpContext {
    DWORD64 P1Home;
    DWORD64 P2Home;
    DWORD64 P3Home;
    DWORD64 P4Home;
    DWORD64 P5Home;
    DWORD64 P6Home;
    DWORD ContextFlags;
    DWORD MxCsr;
    WORD SegCs;
    WORD SegDs;
    WORD SegEs;
    WORD SegFs;
    WORD SegGs;
    WORD SegSs;
    DWORD EFlags;
    DWORD64 Dr0;
    DWORD64 Dr1;
    DWORD64 Dr2;
    DWORD64 Dr3;
    DWORD64 Dr6;
    DWORD64 Dr7;
    DWORD64 Rax;
    DWORD64 Rcx;
    DWORD64 Rdx;
    DWORD64 Rbx;
    DWORD64 Rsp;
    DWORD64 Rbp;
    DWORD64 Rsi;
    DWORD64 Rdi;
    DWORD64 R8;
    DWORD64 R9;
    DWORD64 R10;
    DWORD64 R11;
    DWORD64 R12;
    DWORD64 R13;
    DWORD64 R14;
    DWORD64 R15;
    DWORD64 Rip;
    XMM_SAVE_AREA32 FltSave;
    XMM_SAVE_AREA32 FloatSave;
    M128A Header[2];
    M128A Legacy[8];
    M128A Xmm0;
    M128A Xmm1;
    M128A Xmm2;
    M128A Xmm3;
    M128A Xmm4;
    M128A Xmm5;
    M128A Xmm6;
    M128A Xmm7;
    M128A Xmm8;
    M128A Xmm9;
    M128A Xmm10;
    M128A Xmm11;
    M128A Xmm12;
    M128A Xmm13;
    M128A Xmm14;
    M128A Xmm15;
    M128A VectorRegister[26];
    DWORD64 VectorControl;
    DWORD64 DebugControl;
    DWORD64 LastBranchToRip;
    DWORD64 LastBranchFromRip;
    DWORD64 LastExceptionToRip;
    DWORD64 LastExceptionFromRip;
} BpContext;

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

                    // Unload all the hooks
                    for (auto& hook : scriptData.bpHooks) {
                        hook->Disable();
                    }

                    scriptData.bpHooks.clear();
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

    lua["find_sig"] = [&](std::string sig, std::string mask) {
        return (uintptr_t)JAPI_FindSignature(sig.c_str(), mask.c_str());
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

    lua["hook_func"] = [&](uintptr_t target, sol::function callback) {
        // This is fucking stupid, but i do not give a smallest fuck
        scriptData->bpHooks.push_back(std::make_unique<BreakpointHook>(target, callback, [&](PEXCEPTION_POINTERS info, sol::function cb) -> void {
            // Create a new table
            sol::table data = lua.create_table();
            
            // This fucking sucks but i dont care
            data["Dr0"] = info->ContextRecord->Dr0;
            data["Dr1"] = info->ContextRecord->Dr1;
            data["Dr2"] = info->ContextRecord->Dr2;
            data["Dr3"] = info->ContextRecord->Dr3;
            data["Dr6"] = info->ContextRecord->Dr6;
            data["Dr7"] = info->ContextRecord->Dr7;
            data["Rax"] = info->ContextRecord->Rax;
            data["Rcx"] = info->ContextRecord->Rcx;
            data["Rdx"] = info->ContextRecord->Rdx;
            data["Rbx"] = info->ContextRecord->Rbx;
            data["Rsp"] = info->ContextRecord->Rsp;
            data["Rbp"] = info->ContextRecord->Rbp;
            data["Rsi"] = info->ContextRecord->Rsi;
            data["Rdi"] = info->ContextRecord->Rdi;
            data["R8"] = info->ContextRecord->R8;
            data["R9"] = info->ContextRecord->R9;
            data["R10"] = info->ContextRecord->R10;
            data["R11"] = info->ContextRecord->R11;
            data["R12"] = info->ContextRecord->R12;
            data["R13"] = info->ContextRecord->R13;
            data["R14"] = info->ContextRecord->R14;
            data["R15"] = info->ContextRecord->R15;
            data["Rip"] = info->ContextRecord->Rip;
            data["Xmm0"] = info->ContextRecord->Xmm0;
            data["Xmm1"] = info->ContextRecord->Xmm1;
            data["Xmm2"] = info->ContextRecord->Xmm2;
            data["Xmm3"] = info->ContextRecord->Xmm3;
            data["Xmm4"] = info->ContextRecord->Xmm4;
            data["Xmm5"] = info->ContextRecord->Xmm5;
            data["Xmm6"] = info->ContextRecord->Xmm6;
            data["Xmm7"] = info->ContextRecord->Xmm7;
            data["Xmm8"] = info->ContextRecord->Xmm8;
            data["Xmm9"] = info->ContextRecord->Xmm9;
            data["Xmm10"] = info->ContextRecord->Xmm10;
            data["Xmm11"] = info->ContextRecord->Xmm11;
            data["Xmm12"] = info->ContextRecord->Xmm12;
            data["Xmm13"] = info->ContextRecord->Xmm13;
            data["Xmm14"] = info->ContextRecord->Xmm14;
            data["Xmm15"] = info->ContextRecord->Xmm15;

            // Actually call the callback
            bool result = cb(data);

            // Again this sucks
            info->ContextRecord->Dr0 = data["Dr0"];
            info->ContextRecord->Dr1 = data["Dr1"];
            info->ContextRecord->Dr2 = data["Dr2"];
            info->ContextRecord->Dr3 = data["Dr3"];
            info->ContextRecord->Dr6 = data["Dr6"];
            info->ContextRecord->Dr7 = data["Dr7"];
            info->ContextRecord->Rax = data["Rax"];
            info->ContextRecord->Rcx = data["Rcx"];
            info->ContextRecord->Rdx = data["Rdx"];
            info->ContextRecord->Rbx = data["Rbx"];
            info->ContextRecord->Rsp = data["Rsp"];
            info->ContextRecord->Rbp = data["Rbp"];
            info->ContextRecord->Rsi = data["Rsi"];
            info->ContextRecord->Rdi = data["Rdi"];
            info->ContextRecord->R8 = data["R8"];
            info->ContextRecord->R9 = data["R9"];
            info->ContextRecord->R10 = data["R10"];
            info->ContextRecord->R11 = data["R11"];
            info->ContextRecord->R12 = data["R12"];
            info->ContextRecord->R13 = data["R13"];
            info->ContextRecord->R14 = data["R14"];
            info->ContextRecord->R15 = data["R15"];
            info->ContextRecord->Rip = data["Rip"];
            info->ContextRecord->Xmm0 = data["Xmm0"];
            info->ContextRecord->Xmm1 = data["Xmm1"];
            info->ContextRecord->Xmm2 = data["Xmm2"];
            info->ContextRecord->Xmm3 = data["Xmm3"];
            info->ContextRecord->Xmm4 = data["Xmm4"];
            info->ContextRecord->Xmm5 = data["Xmm5"];
            info->ContextRecord->Xmm6 = data["Xmm6"];
            info->ContextRecord->Xmm7 = data["Xmm7"];
            info->ContextRecord->Xmm8 = data["Xmm8"];
            info->ContextRecord->Xmm9 = data["Xmm9"];
            info->ContextRecord->Xmm10 = data["Xmm10"];
            info->ContextRecord->Xmm11 = data["Xmm11"];
            info->ContextRecord->Xmm12 = data["Xmm12"];
            info->ContextRecord->Xmm13 = data["Xmm13"];
            info->ContextRecord->Xmm14 = data["Xmm14"];
            info->ContextRecord->Xmm15 = data["Xmm15"];

            // If false we should skip calling the original function
            if(!result) {
                info->ContextRecord->Rip = AllocatedRetInstruction();
            }
        }));

        return scriptData->bpHooks.size();
    };

    lua["unhook_func"] = [&](uint16_t hookId) {
        scriptData->bpHooks[hookId]->Disable();
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