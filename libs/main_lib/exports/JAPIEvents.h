//
// Created by kapil on 27.12.2025.
//

#ifndef JAPI_PRELOAD_JAPIEVENTS_H
#define JAPI_PRELOAD_JAPIEVENTS_H

#include <stdint.h> // NOLINT(*-deprecated-headers)
#include <windows.h>

typedef bool (*EventCallback)(void* data);

enum ButtonState {
    J_BUTTONSTATE_DOWN = WM_KEYDOWN,
    J_BUTTONSTATE_UP = WM_KEYUP
};

// "KeyboardEvent"
struct KeyboardEvent {
    uint64_t key{};
    ButtonState state{};
};

// "HWNDCallbackEvent" - CANCELLABLE
struct HWNDCallbackEvent {
    HWND hwnd;
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;
};

// "ImGUIRenderEvent"
struct ImGUIRenderEvent {
    /* No Data */
};

// "JAPILateInitEvent"
struct JAPILateInitEvent {
    /* No Data */
};

#endif //JAPI_PRELOAD_JAPIEVENTS_H