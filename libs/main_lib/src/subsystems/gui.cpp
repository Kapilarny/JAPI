//
// Created by kapil on 1.12.2025.
//

#include "gui.h"

#include "logger.h"
#include "kiero/kiero.h"

#include <imgui.h>
#include <ranges>

#include "japi.h"

ImVec4 GetChromaColor() {
    float r, g, b;
    ImGui::ColorConvertHSVtoRGB((ImGui::GetFrameCount() % 360) / 360.f, 1.0f, 1.0f, r, g, b); // NOLINT(*-narrowing-conversions)

    return {r, g, b, 1.0f};
}

void gui_manager::update() {
    ImGui::Begin("JAPI (F1 to toggle)");

    if (ImGui::BeginTabBar("tabs")) {
        for (const auto& [name, func] : std::ranges::reverse_view(registered_tab_items)) {
            if (ImGui::BeginTabItem(name.c_str())) {
                func();

                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }


    ImGui::End();
}

void gui_manager::register_tab_item(const std::string& item_name, const std::function<void()>& func) {
    auto contains = [=](const std::pair<std::string, std::function<void()>>& x) {return x.first == item_name;};
    ASSERT(std::views::filter(registered_tab_items, contains).empty(),
        "Trying to insert a tab item which name already exists within the registered tab items!");

    registered_tab_items.emplace_back(item_name, func);
}

void gui_manager::init() {
    auto result = kiero::init(kiero::RenderType::D3D11);
    if (result != kiero::Status::Success) {
        JFATAL("Failed to initialize kiero GUI subsystem! Error code: %d", result);
        return;
    }

    instance = std::unique_ptr<gui_manager>(new gui_manager());
}

gui_manager::gui_manager() {
    init_native_hooks();

    register_tab_item("Credits", [] {
        ImGui::Text("Build: " __DATE__ " || " __TIME__);

        ImGui::TextColored(GetChromaColor(), "JAPI v%s || Made by Kapilarny :)", JAPI_VERSION);
        ImGui::Spacing();

        ImGui::Text("Huge thanks to: ");

        const char* ppl[] = {"Kojo Bailey", "yeeeeeeee.", "Hydra", "Damn.Broh", "justcamtro", "moeru", "Jake"};
        for(auto person : ppl) {
            ImGui::Bullet(); ImGui::TextColored(GetChromaColor(), "%s", person);
        }
    });

    register_tab_item("Test", [] {
        if (ImGui::Button("Restart Game")) {
            exit(67);
        }
    });
}

gui_manager& gui_manager::get() {
    if (instance == nullptr) {
        JFATAL("Trying to access gui_manager before initialization!");
    }

    return *instance;
}
