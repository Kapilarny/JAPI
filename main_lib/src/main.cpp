//
// Created by kapil on 1.12.2025.
//

#include "internal_japi.h"
#include "japi.h"

#include "subsystems/logger.h"

void internal_japi_initialize(HMODULE h_module) {
    japi::initialize(h_module);

    // Spawn the thread to run japi
    const HANDLE thread = CreateThread(nullptr, 0,
        (LPTHREAD_START_ROUTINE) japi::run_thread, h_module, 0, nullptr);

    if (thread) {
        CloseHandle(thread);
    }
}
