#include "logger.h"

#include <cstdint>
#include <windows.h>
#include <fstream>

static std::ofstream glob_log_file;

void logger::init(const std::string& logger_name) {
    // Set global logger name
    JLOGGERNAME = logger_name;

    // Open log file
    glob_log_file.open(JLOGGERNAME + ".log", std::ios::out | std::ios::trunc);
    if (!glob_log_file.is_open()) {
        ERROR_AND_QUIT("Failed to open log file %s.log for writing!", JLOGGERNAME.c_str());
    }
}

void error_box(const std::string &message, const std::string &title) {
    MessageBoxA(NULL, message.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
}

void logger::spawn_console() {
    AllocConsole();
    // SetConsoleTitle("JAPISigner);
    SetConsoleTitle((JLOGGERNAME + " Console").c_str());
    freopen_s((FILE **)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE **)stdin, "CONIN$", "r", stdin);
    freopen_s((FILE **)stderr, "CONOUT$", "w", stderr);
}

void log_output_with_fmt(LogLevel level, const std::string &message, std::string plugin_guid, ...) {
    va_list args;
    va_start(args, plugin_guid);

    log_output_va_list(level, message, plugin_guid, args);

    va_end(args);
}

void log_output_va_list(LogLevel level, const std::string &message, const std::string &plugin_guid, const va_list args) {
    static const std::string level_strings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: " };
    bool isError = level < 2;

    char buffer[10000];

    // Format the string and store it in the buffer
    vsnprintf(buffer, 10000, message.c_str(), args);

    // Add the plugin GUID to the message
    std::string finalMessage = "[" + plugin_guid + "] " + level_strings[level] + std::string(buffer) + "\n";

    // Write to log file
    glob_log_file << finalMessage;
    glob_log_file.flush();

    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static uint8_t levels[6] = { 64, 4, 6, 2, 1, 8 };

    // Fancy console output
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(console_handle, levels[level]);
    OutputDebugStringA(finalMessage.c_str());
    uint64_t length = strlen(finalMessage.c_str());
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), finalMessage.c_str(), (DWORD)length, number_written, 0);
}
