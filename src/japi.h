#pragma once

#include <memory>
#include <vector>

#include "mod.h"

class JAPI {
public:
    static void Init(HINSTANCE hinstDLL);

    static uint64_t GetASBRModuleBase();
    static HANDLE GetASBRProcessHandle();
    static std::string GetModGUID(HANDLE modHandle);
private:
    JAPI() = default;

    void LoadMods();
    void InitWindowsVars();

    uint64_t asbrModuleBase;
    HINSTANCE hinstDLL;
    HWND hwnd;
    DWORD pid;
    HANDLE processHandle;
    std::vector<ModData> mods;

    static inline std::unique_ptr<JAPI> instance;    
};