#include "kiero.h"
#include "subsystems/gui.h"
#include <logger.h>

#include <d3d11.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "JAPIEvents.h"
#include "subsystems/events.h"
#include "subsystems/hook.h"

static WNDPROC oWndProc = NULL;
static bool shouldWindowBeOpen = true;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK hkWindowProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	// ImGUI Handler
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam) > 0) return 1;

	// JAPI Handler
	{
		HWNDCallbackEvent event{hwnd, uMsg, wParam, lParam};
		auto result = event_manager::get().transmit_event_cancellable("HWNDCallbackEvent", &event);

		if(result) {
			return 0;
		}
	}

	if(uMsg == WM_KEYUP && wParam == VK_F1) shouldWindowBeOpen = !shouldWindowBeOpen;

	switch(uMsg) {
		case WM_KEYUP:
		case WM_KEYDOWN: {
			KeyboardEvent event{ wParam, (ButtonState)uMsg };

			event_manager::get().transmit_event_cancellable("KeyboardEvent", &event);
		} break;

		default: break;
	}

	// Original Game Handler
	return ::CallWindowProcA(oWndProc, hwnd, uMsg, wParam, lParam);
}

typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = NULL;

typedef UINT (__stdcall *GetRawInputData_t)(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);
GetRawInputData_t oGetRawInputData = nullptr;

UINT __stdcall hkGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader)
{
	auto result = oGetRawInputData(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);

	// Check if we should block the input because of ImGUI
	if(ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
		return 0;
	}

	return result;
}

long __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
	static bool init = false;

	if (!init) {
		DXGI_SWAP_CHAIN_DESC desc;
		pSwapChain->GetDesc(&desc);

		ID3D11Device* device;
		pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device);

		ID3D11DeviceContext* context;
		device->GetImmediateContext(&context);

		// Hook WndProc
		oWndProc = (WNDPROC)::SetWindowLongPtr(desc.OutputWindow, GWLP_WNDPROC, (LONG_PTR)hkWindowProc);

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(desc.OutputWindow);
		ImGui_ImplDX11_Init(device, context);

		init = true;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if(shouldWindowBeOpen) {
		gui_manager::get().update();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(pSwapChain, SyncInterval, Flags);
}


void gui_manager::init_native_hooks() {
    ASSERT(kiero::bind(8, (void**)&oPresent, (void*)hkPresent11) == kiero::Status::Success, "Failed to bind Present hook!");

	// Hook GetRawInputData
	HMODULE hUser32 = LoadLibraryA("user32.dll");
	oGetRawInputData = (GetRawInputData_t)GetProcAddress(hUser32, "GetRawInputData");

	JAPIHookMeta h = {
		.target = (uint64_t)oGetRawInputData,
		.detour = (void*)hkGetRawInputData,
		.original = &oGetRawInputData,
		.name = "GetRawInputData",
		.game_function = false
	};

	hook_manager::get().register_hook(h);
}
