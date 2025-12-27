//
// Created by kapil on 1.12.2025.
//

#include "japi.h"

#include "subsystems/logger.h"

#include <imgui.h>

#include "subsystems/events.h"
#include "subsystems/gui.h"
#include "subsystems/hook.h"

japi::japi() : japi_cfg(config::load("japi/config/JAPI.cfg")){
    // Initialize logger
    logger::init();

    if (japi_cfg.bind("spawn_console", false)) {
        logger::spawn_console();
    }

    module_base = GetModuleHandleA(nullptr);

    event_manager::init();
    hook_manager::init();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void japi::run() { // NOLINT(*-convert-member-functions-to-static)
    gui_manager::init();

    JINFO("Fully initialized!");
    fully_initialized = true;
    for (const auto& cb : post_init_callbacks) cb();

    JAPILateInitEvent e{};
    event_manager::get().transmit_event("JAPILateInitEvent", &e);
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

void japi::register_post_init_callback(std::function<void()> cb) {
    post_init_callbacks.emplace_back(std::move(cb));
}

void japi::initialize(HMODULE h_module) {
    if (instance != nullptr) {
        // Create an error box
        MessageBoxA(nullptr, "Initializing JAPI instance multiple times!", "JoJoAPI Error", MB_OK | MB_ICONERROR);
        return;
    }

    instance = std::unique_ptr<japi>(new japi());

    modloader::init(); // Must be after japi instance creation
}
