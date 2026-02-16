//
// Created by kapil on 12.02.2026.
//
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "binary_file.h"
#include "crypt.h"
#include "downloader.h"
#include "logger.h"
#include "process.h"

#include "defines.h"
#include "launcher.h"

int main() {
    // Init logger
    logger::init("JAPILauncher", "japi/logs");

    try {
        launcher l;
        l.run();
    } catch (const std::exception& e) {
        ERROR_AND_QUIT(e.what());
    }

    JINFO("Shutting down launcher...");
}
