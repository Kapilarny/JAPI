//
// Created by kapil on 16.02.2026.
//

#ifndef JAPI_LAUNCHER_H
#define JAPI_LAUNCHER_H
#include <ctime>

#include "config.h"

class launcher {
public:
    launcher();

    void run();
private:
    void check_for_updates();
    void install_japi();
    void cleanup_old_files();
    void launch_game();

    config _cfg;
};

#endif //JAPI_LAUNCHER_H