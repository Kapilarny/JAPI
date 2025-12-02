//
// Created by kapil on 1.12.2025.
//

#ifndef JAPI_JAPI_H
#define JAPI_JAPI_H

#include <memory>
#include <windows.h>

#include "subsystems/config.h"
#include "subsystems/modloader.h"

class japi {
public:
    static void initialize(HMODULE h_module);
    static void run_thread(HINSTANCE h_inst);

    static japi& get();

    [[nodiscard]] HMODULE get_module_base() const { return module_base; }
private:
    japi();

    void run();

    static inline std::unique_ptr<japi> instance = nullptr;

    config japi_cfg;
    HMODULE module_base;
};

#endif //JAPI_JAPI_H