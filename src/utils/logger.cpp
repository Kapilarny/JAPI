#include "Logger.h"

#include <Windows.h>
#include <string>
#include <fstream>

// Log File
static std::ofstream logFile;

void LogOutput(LogLevel level, const std::string& message, const std::string& pluginGUID) {
    static const std::string level_strings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: " };
    bool isError = level < 2;

    // Add the plugin GUID to the message
    std::string finalMessage = "[" + pluginGUID + "] " + level_strings[level] + message + "\n";

    if (!logFile.is_open()) {
        logFile.open("JojoAPI.log", std::ios::out | std::ios::trunc);
    }

    // Write to log file
    logFile << finalMessage;
    logFile.flush();

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