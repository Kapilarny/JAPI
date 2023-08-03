#include "hooks.h"

#include <MinHook.h>

#include "utils/logger.h"

bool HookFunction(Hook* hook) {
    MH_STATUS status = MH_CreateHook(hook->target, hook->detour, (LPVOID*) hook->original);
    if (status != MH_OK) {
        JERROR("Failed to create hook for function" + hook->name);
        JERROR("Error: " + std::to_string(status));
        return false;
    }

    status = MH_EnableHook(hook->target);
    if (status != MH_OK) {
        JERROR("Failed to enable hook for function" + hook->name);
        return false;
    }

    return true;
}

bool UnhookFunction(Hook* hook) {
    MH_STATUS status = MH_DisableHook(hook->target);
    if (status != MH_OK) {
        return false;
    }

    status = MH_RemoveHook(hook->target);
    if (status != MH_OK) {
        return false;
    }

    return true;
}