//
// Created by kapil on 1.12.2025.
//

#include "hook.h"

#include "japi.h"
#include "logger.h"
#include "MinHook.h"

void hook_manager::init() {
    if (instance) {
        JERROR("Trying to reinitialize hook_manager!");
        return;
    }

    if (MH_Initialize() != MH_OK) {
        JFATAL("Failed to initialize MinHook!");
    }

    instance = std::unique_ptr<hook_manager>(new hook_manager());
}

hook_manager& hook_manager::get() {
    if (instance == nullptr) {
        JFATAL("Trying to access hook_manager before initialization!");
    }

    return *instance;
}

JAPIHookHandle hook_manager::register_hook(JAPIHookMeta hook) {
    if (hook.game_function) {
        hook.target += (uint64_t)japi::get().get_module_base();
    }

    MH_STATUS status = MH_CreateHook((LPVOID)hook.target, hook.detour, (LPVOID*) hook.original);
    if (status != MH_OK) {
        JERROR("Failed to create hook for function: %s", hook.name);
        JERROR("Error: %d", status);
        return 0;
    }

    status = MH_EnableHook((LPVOID)hook.target);
    if (status != MH_OK) {
        JERROR("Failed to enable hook for function: %s", hook.name);
        return 0;
    }

    JAPIHookHandle result = 0;
    if (!hook_stack.empty()) {
        result = hook_stack.top();
        hook_stack.pop();

        hooks[result] = hook;
    } else {
        hooks.push_back(hook);
        result = hooks.size()-1;
    }

    return result+1;
}

bool hook_manager::unregister_hook(JAPIHookHandle hook_handle) {
    auto hook_idx = hook_handle-1;
    if (hooks.size() <= hook_idx) {
        JERROR("Trying to deregister a hook with an invalid hook handle!");
        return false;
    }

    // Remove Hook
    auto status = MH_DisableHook((LPVOID)hooks[hook_idx].target);
    if (status != MH_OK) {
        JERROR("Failed to disable hook for function: %s", hooks[hook_idx].name);
        return false;
    }

    status = MH_RemoveHook((LPVOID)hooks[hook_idx].target);
    if (status != MH_OK) {
        JERROR("Failed to remove hook for function: %s", hooks[hook_idx].name);
        return false;
    }

    hooks[hook_idx] = {}; // We consider hooks with a null target empty hooks
    hook_stack.push(hook_idx); // Recycle index

    return true;
}