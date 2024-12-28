#include "japi.h"
#include "kiero.h"
#include "events/event.h"
#include "mods/mod_manager.h"
#include "utils/logger.h"

#if KIERO_INCLUDE_D3D11

#include "d3d11_impl.h"
#include <d3d11.h>
#include <assert.h>

#include "win32_impl.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"

typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = NULL;

static bool shouldWindowBeOpen = true;

ImVec4 HSVtoRGB(float h, float s, float v)
{
	int i = static_cast<int>(h * 6);
	float f = h * 6 - i;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);

	switch (i % 6) {
		case 0: return {v, t, p, 1.0f};
		case 1: return {q, v, p, 1.0f};
		case 2: return {p, v, t, 1.0f};
		case 3: return {p, q, v, 1.0f};
		case 4: return {t, p, v, 1.0f};
		case 5: return {v, p, q, 1.0f};
		default: return {1.0f, 1.0f, 1.0f, 1.0f}; // Fallback to white
	}
}

ImVec4 GetChromaColor() {
	return HSVtoRGB((ImGui::GetFrameCount() % 360) / 360.0f, 1.0f, 1.0f);
}

void imgui_update() {
	// ImGUI Render
	ImGui::Begin("JAPI (F1 to toggle)", &shouldWindowBeOpen);

	if(ImGui::BeginTabBar("tabs")) {
		if(ImGui::BeginTabItem("Mods")) {
			// TODO: Implement mods tab
			mod_manager::get_instance()->draw_imgui_mods_tab();

			ImGui::EndTabItem();
		}

		if(ImGui::BeginTabItem("Credits")) {
			ImGui::Text("Build: " __DATE__ " || " __TIME__);

			ImGui::TextColored(GetChromaColor(), "JAPI v%s || Made by Kapilarny :)", JAPI_VERSION);
			ImGui::Spacing();

			ImGui::Text("Huge thanks to: ");

			const char* ppl[] = {"Kojo Bailey", "yeeeeeeee.", "Hydra", "Damn.Broh", "justcamtro", "moeru", "Jake"};
			for(auto person : ppl) {
				ImGui::Bullet(); ImGui::TextColored(GetChromaColor(), "%s", person);
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGUIRenderEvent e{};
	event_transmitter::transmit_event("ImGUIRenderEvent", &e);

	ImGui::End();

	// ImGUI Render End
}

long __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
	static bool init = false;

	if (!init)
	{
		DXGI_SWAP_CHAIN_DESC desc;
		pSwapChain->GetDesc(&desc);

		ID3D11Device* device;
		pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device);

		ID3D11DeviceContext* context;
		device->GetImmediateContext(&context);

		init_win32_hooks(desc.OutputWindow);

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(desc.OutputWindow);
		ImGui_ImplDX11_Init(device, context);

		init = true;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if(shouldWindowBeOpen) {
		imgui_update();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(pSwapChain, SyncInterval, Flags);
}

void toggle_imgui_window() {
	// JTRACE("Toggling imgui window!");
	shouldWindowBeOpen = !shouldWindowBeOpen;
}

void init_d3d11_hooks()
{
	// JTRACE("Hooking d3d11!");
	assert(kiero::bind(8, (void**)&oPresent, (void*)hkPresent11) == kiero::Status::Success);
}

#endif // KIERO_INCLUDE_D3D11