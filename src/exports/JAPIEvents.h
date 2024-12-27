#ifndef JAPIEVENTS_H
#define JAPIEVENTS_H

#include <cstdint>
#include <windows.h>

typedef bool (*EventCallback)(void* data);

enum ButtonState {
    BUTTONSTATE_DOWN = WM_KEYDOWN,
    BUTTONSTATE_UP = WM_KEYUP
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

#endif //JAPIEVENTS_H