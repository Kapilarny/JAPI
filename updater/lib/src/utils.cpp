#include "utils.h"

#include <Windows.h>
#include <stdio.h>

#include "logger.h"

void LaunchGame() {
    // Run ASBR.exe
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Get the current directory
    char current_directory[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, current_directory);

    // Append the ASBR.exe path
    strcat(current_directory, "\\ASBR.exe");

    JINFO(current_directory);

    // Start the child process.
    if(!CreateProcessA(
        NULL, // No module name
        current_directory, // Command line
        NULL, // Process handle not inheritable
        NULL, // Thread handle not inheritable
        FALSE, // Set handle inheritance to FALSE
        0, // No creation flags
        NULL, // Use parent's environment block
        NULL, // Use parent's starting directory
        &si, // Pointer to STARTUPINFO structure
        &pi // Pointer to PROCESS_INFORMATION structure
    )) {
        JFATAL("CreateProcess failed (" + std::to_string(GetLastError()) + ")");
        return;
    }
}