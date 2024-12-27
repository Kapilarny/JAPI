//
// Created by user on 27.12.2024.
//

#ifndef HOOKS_H
#define HOOKS_H

#include "exports/JoJoAPI.h"

namespace hooks {
    bool hook_function(const JAPIHook& hook);
    bool unhook_function(const JAPIHook& hook);
}

#endif //HOOKS_H
