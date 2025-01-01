//
// Created by user on 27.12.2024.
//

#include "japi.h"

#include <imgui.h>
#include <MinHook.h>

#include <Windows.h>
#include <psapi.h>

#include "events/event.h"
#include "kiero/d3d11_impl.h"
#include "kiero/kiero.h"
#include "mods/mod_manager.h"
#include "utils/logger.h"

void japi::initialize(HINSTANCE dll_h_inst) {
    instance = std::unique_ptr<japi>(new japi());
    instance->h_inst = GetModuleHandle(NULL);
    instance->module_base = (uint64_t) GetModuleHandle(NULL);

    instance->find_game_type();

    // Check if we should enable debug console
    if (instance->japi_cfg.bind<bool>("spawn_console", false)) {
        AllocConsole();
        SetConsoleTitle("JoJoAPI Console");
        FILE* file = nullptr;
        freopen_s(&file, "CONOUT$", "w", stdout);
    }

    if(!instance->japi_cfg.bind<bool>("asked_for_default_plugins", false)) {
        // Create a message box
        auto result = MessageBoxA(nullptr, "Would you like to install game-specific default plugins?", "JoJoAPI", MB_YESNO | MB_ICONQUESTION);

        if(result == IDYES) {
            instance->download_default_plugins = true;
        }

        instance->japi_cfg.set("asked_for_default_plugins", true);
    }

    JINFO("Loaded JAPI v%s (game_type %s)", JAPI_VERSION, game_type_to_string(instance->type).c_str());

    // Init MinHook
    if (MH_Initialize() != MH_OK) {
        JFATAL("Failed to initialize MinHook");
        return;
    }

    // Get the module size
    MODULEINFO info;
    GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(0), &info, sizeof(MODULEINFO));
    instance->module_size = info.SizeOfImage;

    // Init events
    event_transmitter::init();
    mod_manager::init();

    JINFO("Initialized JoJoAPI!");
}

void japi::run_thread(HINSTANCE h_inst) {
    // Load kiero
    auto result = kiero::init(kiero::RenderType::D3D11);
    if (result != kiero::Status::Success) {
        JFATAL("Failed to initialize D3D11 hooks! (%d)", result);
        return;
    }

    init_d3d11_hooks();

    JAPILateInitEvent e{};
    event_transmitter::transmit_event("JAPILateInitEvent", &e);

    // some update loop could be here, but we don't need it for now
}

void japi::find_game_type() {
    // Get executable path
    char path[MAX_PATH];
    GetModuleFileName(instance->h_inst, path, MAX_PATH);

    // Extract name of the module
    char* module_name = strrchr(path, '\\') + 1;

    // Extract name of the module without extension
    char* module_name_no_ext = strtok(module_name, ".");

    type = string_to_game_type(module_name_no_ext);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            japi::initialize(hinstDLL);

            const HANDLE thread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE) japi::run_thread, hinstDLL, 0, nullptr);
            if (thread) {
                CloseHandle(thread);
            }

            break;
    }
    return TRUE;
}
