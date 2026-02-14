//
// Created by kapil on 1.12.2025.
//

#ifndef JAPI_PRELOAD_HOOK_H
#define JAPI_PRELOAD_HOOK_H

#include <memory>
#include <stack>
#include <vector>

#include "JoJoAPI.h"

class hook_manager {
public:
    static void init();

    static hook_manager& get();

    JAPIHookHandle register_hook(JAPIHookMeta hook);

    bool unregister_hook(JAPIHookHandle hook_handle);
private:
    hook_manager() = default; // TODO: Keep track of hooks and their states

    static inline std::unique_ptr<hook_manager> instance = nullptr;

    std::vector<JAPIHookMeta> hooks;
    std::stack<JAPIHookHandle> hook_stack; // Recycles unused indexes
};

#endif //JAPI_PRELOAD_HOOK_H