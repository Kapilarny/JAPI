//
// Created by kapil on 12.02.2026.
//
#include "process.h"

#include <filesystem>
#include <stdexcept>
#include <tlhelp32.h>

process::process(const char *exe_path, const char *working_dir) {
    _exe_path = exe_path;
    _working_dir = working_dir ? working_dir : std::filesystem::current_path().string();
}

bool process::is_process_running(const char *process_name) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe{};
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &pe)) {
        do {
            if (strcmp(pe.szExeFile, process_name) == 0) {
                CloseHandle(snapshot);
                return true;
            }
        } while (Process32Next(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return false;
}

bool process::is_running() const {
    DWORD exit_code;
    if (!GetExitCodeProcess(_pi.hProcess, &exit_code)) {
        throw std::runtime_error("process::is_running - Failed to get exit code");
    }

    return exit_code == STILL_ACTIVE;
}

void process::inject_dll(const char *dll_path) const {
    if (!_created) {
        throw std::runtime_error("process::inject_dll - Cannot inject DLL into a process that has not been created");
    }

    if (!is_running()) {
        throw std::runtime_error("process::inject_dll - Cannot inject DLL into a process that is not running");
    }

    if (!std::filesystem::exists(dll_path)) {
        throw std::runtime_error("process::inject_dll - DLL file does not exist (" + std::string(dll_path) + ")");
    }

    const auto load_lib = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!load_lib) {
        throw std::runtime_error("process::inject_dll - Failed to get address of LoadLibraryA");
    }

    auto remote_mem = VirtualAllocEx(
        _pi.hProcess,
        0,
        strlen(dll_path)+1,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    if (!remote_mem) {
        throw std::runtime_error("process::inject_dll - Failed to allocate memory in remote process");
    }

    // Write the DLL path to the allocated memory
    if (!WriteProcessMemory(
        _pi.hProcess,
        remote_mem,
        dll_path,
        strlen(dll_path)+1,
        nullptr
    )) {
        VirtualFreeEx(_pi.hProcess, remote_mem, 0, MEM_RELEASE);
        throw std::runtime_error("process::inject_dll - Failed to write DLL path to remote process memory");
    }

    HANDLE thread = CreateRemoteThread(
        _pi.hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)load_lib,
        (LPVOID)remote_mem,
        0,
        NULL
    );

    if (!thread) {
        VirtualFreeEx(_pi.hProcess, remote_mem, 0, MEM_RELEASE);
        throw std::runtime_error("process::inject_dll - Failed to create remote thread");
    }

    WaitForSingleObject(thread, INFINITE);

    VirtualFreeEx(_pi.hProcess, remote_mem, 0, MEM_RELEASE);
}

void process::restart() {
    cleanup();
    create();
}

void process::resume(const bool block_until_exit) const {
    if (!_created) {
        throw std::runtime_error("process::resume - Cannot resume a process that has not been created");
    }

    if (!ResumeThread(_pi.hThread)) {
        throw std::runtime_error("process::resume - Failed to resume process");
    }

    if (block_until_exit) {
        WaitForSingleObject(_pi.hProcess, INFINITE);
    }
}

DWORD process::get_exit_code() const {
    DWORD exit_code;
    if (!GetExitCodeProcess(_pi.hProcess, &exit_code)) {
        throw std::runtime_error("process::get_exit_code - Failed to get exit code");
    }

    return exit_code;
}


void process::create() {
    _si = {};
    _si.cb = sizeof(_si);
    _pi = {};

    if (is_process_running(_exe_path.c_str())) {
        throw std::runtime_error("process::create - Process is already running");
    }

    if (!CreateProcessA(
        _exe_path.c_str(),
        nullptr, nullptr, nullptr, FALSE,
        CREATE_SUSPENDED,
        nullptr, _working_dir.c_str(), &_si, &_pi)) {
        throw std::runtime_error("process::process - Failed to create process");
    }

    _created = true;
}

void process::cleanup() {
    if (!_created) {
        return;
    }

    TerminateProcess(_pi.hProcess, 0);

    CloseHandle(_pi.hThread);
    CloseHandle(_pi.hProcess);
}
