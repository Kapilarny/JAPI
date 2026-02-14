//
// Created by kapil on 12.02.2026.
//

#ifndef JAPI_PRELOAD_PROCESS_H
#define JAPI_PRELOAD_PROCESS_H

#include <string>
#include <windows.h>

class process {

public:
    explicit process(const char* exe_path, const char* working_dir = nullptr);

    static bool is_process_running(const char* process_name);
    bool is_running() const;

    void inject_dll(const char* dll_path) const;
    void restart();
    void resume(bool block_until_exit) const;

    [[nodiscard]] DWORD get_exit_code() const;
private:
    void create();
    void cleanup();

    std::string _exe_path;
    std::string _working_dir;
    STARTUPINFOA _si{};
    PROCESS_INFORMATION _pi{};
};

#endif //JAPI_PRELOAD_PROCESS_H