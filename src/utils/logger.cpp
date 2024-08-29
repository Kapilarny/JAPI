#include "Logger.h"

#include <Windows.h>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdarg.h>

// Log File
static std::ofstream logFile;

void LogOutputVaList(LogLevel level, const std::string& message, const std::string& pluginGUID, va_list args) {
    static const std::string level_strings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: " };
    bool isError = level < 2;
    
    char* buffer = new char[10000]; // 10k is more than enough

    // Format the string and store it in the buffer
    vsnprintf(buffer, 10000, message.c_str(), args);

    // Add the plugin GUID to the message
    std::string finalMessage = "[" + pluginGUID + "] " + level_strings[level] + std::string(buffer) + "\n";

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

    delete[] buffer;
}

void LogOutputWithFmt(LogLevel level, const std::string& message, const std::string& pluginGUID, ...) {
    va_list args;
    
    // Initialize the argument list
    va_start(args, pluginGUID);

    LogOutputVaList(level, message, pluginGUID, args);
        
    // Clean up the argument list
    va_end(args);
}