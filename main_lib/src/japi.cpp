//
// Created by kapil on 1.12.2025.
//

#include "japi.h"

#include "subsystems/logger.h"

#include <imgui.h>

#include "subsystems/gui.h"
#include "subsystems/hook.h"

japi::japi() : japi_cfg(config::load("japi/config/JAPI.cfg")){
    // Initialize logger
    logger::init();

    if (japi_cfg.bind("spawn_console", false)) {
        logger::spawn_console();
    }

    module_base = GetModuleHandleA(nullptr);

    hook_manager::init();
    modloader::init();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void japi::run() { // NOLINT(*-convert-member-functions-to-static)
    gui_manager::init();
}

void japi::run_thread(HINSTANCE h_inst) {
    if (instance == nullptr) {
        // Create an error box
        MessageBoxA(nullptr, "Trying to run JAPI thread before initialization!", "JoJoAPI Error", MB_OK | MB_ICONERROR);
        return;
    }

    instance->run();
}

japi& japi::get() {
    if (instance == nullptr) {
        // Create an error box
        MessageBoxA(nullptr, "Trying to access JAPI instance before initialization!", "JoJoAPI Error", MB_OK | MB_ICONERROR);
    }

    return *instance;
}

void japi::initialize(HMODULE h_module) {
    if (instance != nullptr) {
        // Create an error box
        MessageBoxA(nullptr, "Initializing JAPI instance multiple times!", "JoJoAPI Error", MB_OK | MB_ICONERROR);
        return;
    }

    instance = std::unique_ptr<japi>(new japi());
}
