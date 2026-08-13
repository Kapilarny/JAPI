//
// Created by kapil on 12.08.2026.
//

#include "input.h"

#include <windows.h>

bool input::query(const std::string& msg, const std::string& caption) {
    return
        MessageBoxA(nullptr, msg.c_str(), caption.c_str(), MB_ICONQUESTION | MB_YESNO) == IDYES;
}
