#include <filesystem>
#include <fstream>
#include <string>
#include <windows.h>

void err(const std::string &message) {
    MessageBoxA(NULL, message.c_str(), "JoJoAPI Error", MB_OK | MB_ICONERROR);
}

void load_japi(HMODULE hModule) {
    auto japi_lib = LoadLibraryA("japi/dlls/JAPI.dll");
    if (!japi_lib) {
        MessageBoxA(NULL, "Failed to load japi.dll!", "JoJoAPI Error", MB_OK | MB_ICONERROR);
        return;
    }
    typedef void (*internal_japi_initialize_t)(HMODULE);
    auto internal_japi_initialize = (internal_japi_initialize_t)GetProcAddress(japi_lib, "internal_japi_initialize");
    if (!internal_japi_initialize) {
        MessageBoxA(NULL, "Failed to find internal_japi_initialize in japi.dll!", "JoJoAPI Error", MB_OK | MB_ICONERROR);
        return;
    }

    internal_japi_initialize(hModule);
}

void preload_library(const std::string &path) {
    const HMODULE lib = LoadLibraryA(path.c_str());
    if (!lib) {
        err("Failed to preload library: " + path);

        // Log the error code
        const DWORD error_code = GetLastError();
        err("Error code: " + std::to_string(error_code));
    }
}

void preload_libraries() {
    // Grab list of libraries from the japi/libs directory
    if (!std::filesystem::exists("japi/dlls/libs/")) {
        err("japi/libs/ directory does not exist!");
        return;
    }

    const std::string preload_list_path = "japi/dlls/lib_load_order.txt";
    if (!std::filesystem::exists(preload_list_path)) {
        err("Preload list file japi/dlls/lib_load_order.txt does not exist!");
        return;
    }

    std::ifstream preload_list_file(preload_list_path);
    std::string line;
    while (std::getline(preload_list_file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Preload the library
        const std::string lib_path = "japi/dlls/libs/" + line;
        preload_library(lib_path);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            preload_libraries();
            load_japi(hModule);
        } break;
    }
    return TRUE;
}
