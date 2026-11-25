//
// Created by kapil on 1.12.2025.
//

#include "hook.h"

#include "japi.h"
#include "logger.h"
#include "MinHook.h"

#include <algorithm>

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

JAPIBatchRegisterHookResult hook_manager::batch_register_hooks(JAPIBatchRegisterHookContainer batch) {
    JAPIBatchRegisterHookResult result{};
    result.count = batch.count;
    result.handles = (JAPIHookHandle*)calloc(batch.count ? batch.count : 1, sizeof(JAPIHookHandle));
    if (!result.handles) {
        result.count = 0;
        return result;
    }

    // Absolute targets, kept per entry so the registry never sees an RVA.
    std::vector<JAPIHookMeta> metas(batch.data, batch.data + batch.count);
    std::vector<bool> created(batch.count, false);

    for (uint32_t i = 0; i < batch.count; i++) {
        JAPIHookMeta& hook = metas[i];
        if (hook.game_function) {
            hook.target += (uint64_t)japi::get().get_module_base();
        }

        if (registered_hook_addresses.contains(hook.target)) {
            JERROR("Trying to hook a function which is already hooked! Rejecting the hook... (Addr: %p)", (void*)hook.target);
            continue;
        }

        const MH_STATUS status = MH_CreateHook((LPVOID)hook.target, hook.detour, (LPVOID*)hook.original);
        if (status != MH_OK) {
            // A second entry for the same target in one batch lands here too (MH_ERROR_ALREADY_CREATED).
            JERROR("Failed to create hook for function: %s (error %d)", hook.name, status);
            continue;
        }
        const MH_STATUS queued = MH_QueueEnableHook((LPVOID)hook.target);
        if (queued != MH_OK) {
            JERROR("Failed to queue up hook for function: %s (error %d)", hook.name, queued);
            MH_RemoveHook((LPVOID)hook.target);
            continue;
        }
        created[i] = true;
    }

    const MH_STATUS status = MH_ApplyQueued();
    if (status != MH_OK) {
        // MinHook stops at the first failure, so some may be live: remove them all (RemoveHook disables first).
        JERROR("Failed to apply batched hooks! (error %d)", status);
        for (uint32_t i = 0; i < batch.count; i++) {
            if (created[i]) {
                MH_RemoveHook((LPVOID)metas[i].target);
            }
        }

        free(result.handles);
        result.handles = nullptr;
        result.count = 0;
        return result;
    }

    for (uint32_t i = 0; i < batch.count; i++) {
        if (!created[i]) continue; // handle stays 0

        registered_hook_addresses.insert(metas[i].target);

        JAPIHookHandle slot = 0;
        if (!hook_stack.empty()) {
            slot = hook_stack.top();
            hook_stack.pop();
            hooks[slot] = metas[i];
        } else {
            hooks.push_back(metas[i]);
            slot = hooks.size() - 1;
        }

        result.handles[i] = slot + 1;
    }

    return result;
}

JAPIBatchHookUnregisterResult hook_manager::batch_unregister_hooks(JAPIBatchUnregisterHookContainer batch) {
    JAPIBatchHookUnregisterResult result{};
    result.count = batch.count;
    result.success = (bool*)calloc(batch.count ? batch.count : 1, sizeof(bool)); // all false
    if (!result.success) {
        result.count = 0;
        return result;
    }

    std::vector<LPVOID> targets(batch.count, nullptr);
    std::vector<size_t> hook_indices(batch.count, hooks.size());

    for (uint32_t i = 0; i < batch.count; i++) {
        const JAPIHookHandle hook_handle = batch.handles[i];
        if (hook_handle == 0 || hook_handle - 1 >= hooks.size()) {
            JERROR("Trying to deregister a hook with an invalid hook handle!");
            continue;
        }

        const size_t hook_idx = hook_handle - 1;
        const auto target = (LPVOID)hooks[hook_idx].target;
        if (std::find(targets.begin(), targets.begin() + i, target) != targets.begin() + i) {
            JERROR("Trying to deregister the same hook more than once in a batch!");
            continue;
        }

        const MH_STATUS status = MH_QueueDisableHook(target);
        if (status != MH_OK) {
            JERROR("Failed to queue up hook for function: %s (error %d)", hooks[hook_idx].name, status);
            continue;
        }

        targets[i] = target;
        hook_indices[i] = hook_idx;
    }

    const MH_STATUS apply_status = MH_ApplyQueued();
    if (apply_status != MH_OK) {
        // Put the queue back, or the next unrelated MH_ApplyQueued switches these hooks off.
        JERROR("Failed to apply batched hook unregistrations! (error %d)", apply_status);

        for (uint32_t i = 0; i < batch.count; i++) {
            if (targets[i]) {
                MH_QueueEnableHook(targets[i]);
            }
        }

        MH_ApplyQueued();
        return result;
    }

    for (uint32_t i = 0; i < batch.count; i++) {
        if (targets[i] == nullptr) continue;
        const size_t hook_idx = hook_indices[i];
        const MH_STATUS status = MH_RemoveHook(targets[i]);
        if (status != MH_OK) {
            JERROR("Failed to remove hook for function: %s (error %d)", hooks[hook_idx].name, status);
            continue;
        }

        registered_hook_addresses.erase(hooks[hook_idx].target);
        hooks[hook_idx] = {};
        hook_stack.push(hook_idx);
        result.success[i] = true;
    }

    return result;
}

JAPIHookHandle hook_manager::register_hook(JAPIHookMeta hook) {
    if (hook.game_function) {
        hook.target += (uint64_t)japi::get().get_module_base();
    }

    if (registered_hook_addresses.contains(hook.target)) {
        JERROR("Trying to hook a function which is already hooked! Rejecting the hook... (Addr: %p)", hook.target);
        return 0;
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

    registered_hook_addresses.insert(hook.target);

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

    registered_hook_addresses.erase(hooks[hook_idx].target);
    hooks[hook_idx] = {}; // We consider hooks with a null target empty hooks
    hook_stack.push(hook_idx); // Recycle index

    return true;
}