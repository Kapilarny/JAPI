//
// Created by user on 27.12.2024.
//

#ifndef JAPI_H
#define JAPI_H

#define JAPI_VERSION "3.1.0"

#include <Windows.h>
#include <memory>

#include "utils/config.h"
#include "utils/game_type.h"

class japi {
public:
    static void initialize(HINSTANCE h_inst);
    static void run_thread(HINSTANCE h_inst);

    static japi& get_instance() { return *instance; }

    uint64_t get_module_base() const { return module_base; }
    uint64_t get_module_size() const { return module_size; }
private:
    japi() = default;

    void find_game_type();

    static inline std::unique_ptr<japi> instance;
    HINSTANCE h_inst{};
    uint64_t module_base{};
    uint64_t module_size{};

    game_type type = game_type::UNKNOWN;

    config japi_cfg = config::load_mod_config("JAPI");
};

#endif //JAPI_H
