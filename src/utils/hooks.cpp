//
// Created by user on 27.12.2024.
//

#include "hooks.h"

#include <MinHook.h>

#include "logger.h"

bool hooks::hook_function(const JAPIHook& hook) {
    MH_STATUS status = MH_CreateHook((LPVOID)hook.target, hook.detour, (LPVOID*) hook.original);
    if (status != MH_OK) {
        JERROR("Failed to create hook for function: %s", hook.name);
        JERROR("Error: %d", status);
        return false;
    }

    status = MH_EnableHook((LPVOID)hook.target);
    if (status != MH_OK) {
        JERROR("Failed to enable hook for function: %s", hook.name);
        return false;
    }

    return true;
}

bool hooks::unhook_function(const JAPIHook& hook) {
    MH_STATUS status = MH_DisableHook((LPVOID)hook.target);
    if (status != MH_OK) {
        JERROR("Failed to disable hook for function: %s", hook.name);
        return false;
    }

    status = MH_RemoveHook((LPVOID)hook.target);
    if (status != MH_OK) {
        JERROR("Failed to remove hook for function: %s", hook.name);
        return false;
    }

    return true;
}
