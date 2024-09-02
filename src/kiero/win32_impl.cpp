//
// Created by user on 01.09.2024.
//

#include "win32_impl.h"
#include "kiero.h"

#include <Windows.h>

#include "d3d11_impl.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "events/event.h"
#include "utils/logger.h"

using namespace ImGui;

static WNDPROC oWndProc = NULL;

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
        auto result = EventTransmitter::TransmitEventCancellable("HWNDCallbackEvent", &event);

        if(result) {
            return 0;
        }
    }

    if(uMsg == WM_KEYUP && wParam == VK_F1) toggle_imgui_window();

    switch(uMsg) {
        case WM_KEYUP:
        case WM_KEYDOWN: {
            KeyboardEvent event{ wParam, (ButtonState)uMsg };

            EventTransmitter::TransmitEvent("KeyboardEvent", &event);
        } break;

        default: break;
    }

    // Original Game Handler
    return ::CallWindowProcA(oWndProc, hwnd, uMsg, wParam, lParam);
}

void init_win32_hooks(void* hwnd)
{
    oWndProc = (WNDPROC)::SetWindowLongPtr((HWND)hwnd, GWLP_WNDPROC, (LONG_PTR)hkWindowProc);
}