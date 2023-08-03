#include "hooks.h"

#include <MinHook.h>

bool HookFunction(Hook* hook) {
    MH_STATUS status = MH_CreateHook(hook->target, hook->detour, &hook->original);
    if (status != MH_OK) {
        return false;
    }

    status = MH_EnableHook(hook->target);
    if (status != MH_OK) {
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