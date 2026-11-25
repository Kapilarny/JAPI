//
// Created by kapil on 1.12.2025.
//

#ifndef JAPI_PRELOAD_HOOK_H
#define JAPI_PRELOAD_HOOK_H

#include <memory>
#include <stack>
#include <vector>
#include <unordered_set>

#include "JoJoAPI.h"

class hook_manager {
public:
    static void init();

    static hook_manager& get();

    JAPIBatchRegisterHookResult batch_register_hooks(JAPIBatchRegisterHookContainer hooks);
    JAPIBatchHookUnregisterResult batch_unregister_hooks(JAPIBatchUnregisterHookContainer hooks);

    JAPIHookHandle register_hook(JAPIHookMeta hook);
    bool unregister_hook(JAPIHookHandle hook_handle);
private:
    hook_manager() = default; // TODO: Keep track of hooks and their states

    static inline std::unique_ptr<hook_manager> instance = nullptr;

    std::unordered_set<uintptr_t> registered_hook_addresses;
    std::vector<JAPIHookMeta> hooks;
    std::stack<JAPIHookHandle> hook_stack; // Recycles unused indexes
};

#endif //JAPI_PRELOAD_HOOK_H