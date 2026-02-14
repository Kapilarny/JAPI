//
// Created by kapil on 1.12.2025.
//

#ifndef JAPI_JAPI_H
#define JAPI_JAPI_H

#define JAPI_VERSION "4.0.0"

#include <memory>
#include <functional>
#include <windows.h>

#include "subsystems/config.h"
#include "subsystems/modloader.h"

class japi {
public:
    static void initialize(HMODULE h_module);
    static void run_thread(HINSTANCE h_inst);

    static japi& get();

    void register_post_init_callback(std::function<void()> cb);

    [[nodiscard]] HMODULE get_module_base() const { return module_base; }
    [[nodiscard]] bool is_initialized() const { return fully_initialized; }
private:
    japi();

    void run();

    static inline std::unique_ptr<japi> instance = nullptr;

    std::vector<std::function<void()>> post_init_callbacks;
    config japi_cfg;
    HMODULE module_base;
    bool fully_initialized = false;
};

#endif //JAPI_JAPI_H