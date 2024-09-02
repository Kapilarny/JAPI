#include "cpk.h"

#include <vector>
#include <string>
#include <algorithm>

#include "japi.h"
#include "utils/config.h"
#include "utils/logger.h"

// This is weird af
struct cpkdata
{
	const char* path;
	__int64 priority;
};

typedef __int64(__fastcall* sub_645828)(unsigned __int64 a1, unsigned __int64 a2);
sub_645828 sub_645828_Original;

// bad practice but whatever
static std::vector<std::string> cpks;
static ModConfig config; 

__int64 __fastcall sub_645828_Hook(unsigned __int64 a1, unsigned __int64 a2)
{
    auto result = sub_645828_Original(a1, a2);
    
    auto loadCpk = (__int64(__fastcall*)(__int64 a, __int64 b))(JAPI::GetASBRModuleBase() + JAPI::GetGameData().functions.CPKLoadFunc);

    for(auto& cpk : cpks) {
        // Change all backslashes to forward slashes
        std::replace(cpk.begin(), cpk.end(), '\\', '/');

        // Get the filename
        auto filename = cpk.substr(cpk.find_last_of("/\\") + 1);

        // Remove the last 4 characters (.cpk)
        auto fileNoExt = filename.substr(0, filename.size() - 4);

        // Check if there exists an .cpkinstall file with the same name
        auto install = "japi\\cpks\\" + fileNoExt + ".cpkinstall";
        if(std::filesystem::exists(install)) {
            // Read the file
            std::ifstream file(install);
            std::string line;
            std::getline(file, line);
            int filePriority = std::stoi(line);

            LOG_TRACE("CPKModLoader", "Found .cpkinstall file for " + filename + " with priority " + std::to_string(filePriority));

            file.close();

            ConfigSet(config.table, filename, filePriority);
            SaveConfig(config);

            // Delete the file
            std::filesystem::remove(install);
        }

        // Check if there exists a .cpk.info file for this cpk
        auto info = "japi\\cpks\\" + fileNoExt + ".cpk.info";
        if(std::filesystem::exists(info)) {
            // Read the file
            std::ifstream file(info);
            // Read 4 bytes
            char buffer[4];
            file.read(buffer, 4);
            
            // Swap to small endian
            std::reverse(&buffer, &buffer + 4);
            int filePriority = *(int*)buffer;

            LOG_TRACE("CPKModLoader", "Found .cpk.info file for " + filename + " with priority " + std::to_string(filePriority));

            file.close();

            ConfigSet(config.table, filename, filePriority);
            SaveConfig(config);

            // Delete the file
            std::filesystem::remove(info);
        }

        // Get the priority
        auto priority = ConfigBind(config.table, filename, 1000);
        SaveConfig(config);

        LOG_TRACE("CPKModLoader", "Loading CPK: " + cpk + " with priority " + std::to_string(priority));

        cpkdata data = { cpk.c_str(), 0 };
        loadCpk((__int64)&data, priority);
    }

    return result;
}

void LoadCPKMods() {
    // Create mod config
    config = GetModConfig("CPKModLoader");

    // Get all files in japi\cpks and load them
    for(auto& p : std::filesystem::directory_iterator("japi\\cpks")) {
        if(p.path().extension() == ".cpk") {
            cpks.push_back(p.path().string());
        }
    }

    // JINFO("What");


    // JTRACE("ASBR Preload Func Addr: %lld", JAPI::GetGameData().functions.CPKPreloadFunc - JAPI_GetASBRModuleBase());
    // JTRACE("ASBR Load Func Addr: %lld", JAPI::GetGameData().functions.CPKLoadFunc - JAPI_GetASBRModuleBase());

    // Hook the CPK loading function
    Hook hook = {
        (void*) JAPI::GetGameData().functions.CPKPreloadFunc,
        (void*) &sub_645828_Hook,
        (void**) &sub_645828_Original,
        "sub_645828"
    };

    JAPI_HookASBRFunction(&hook);
}