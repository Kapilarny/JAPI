//
// Created by kapil on 1.12.2025.
//

#include "gui.h"

#include "logger.h"
#include "kiero/kiero.h"

#include <imgui.h>

void gui_manager::update() {
    ImGui::Begin("JAPI (F1 to toggle)");

    ImGui::End();
}


void gui_manager::init() {
    auto result = kiero::init(kiero::RenderType::D3D11);
    if (result != kiero::Status::Success) {
        JFATAL("Failed to initialize kiero GUI subsystem! Error code: %d", result);
        return;
    }

    instance = std::unique_ptr<gui_manager>(new gui_manager());

    // JAPILateInitEvent e{};
    // event_transmitter::transmit_event("JAPILateInitEvent", &e);
}

gui_manager::gui_manager() {
    init_native_hooks();
}

gui_manager& gui_manager::get() {
    if (instance == nullptr) {
        JFATAL("Trying to access gui_manager before initialization!");
    }

    return *instance;
}
