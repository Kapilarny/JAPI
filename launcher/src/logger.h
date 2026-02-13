//
// Created by kapil on 12.02.2026.
//

#ifndef JAPI_PRELOAD_LOGGER_H
#define JAPI_PRELOAD_LOGGER_H

#include <string>
#include <cstdarg>

typedef enum LogLevel {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} LogLevel;

namespace logger {
    void init();
    void spawn_console();
}

#define ASSERT(expr, msg) \
    do { \
        if (!(expr)) { \
            JFATAL("Assertion failed: %s | Expression: %s", msg, #expr); \
            abort(); \
        } \
    } while (0)

#define ERROR_AND_QUIT(msg, ...) \
    do { \
        JFATAL("Fatal error: %s", msg, ##__VA_ARGS__); \
        error_box(msg, "Fatal Error"); \
        abort(); \
    } while (0)

void error_box(const std::string& message, const std::string& title = "Error");

void log_output_with_fmt(LogLevel level, const std::string& message, std::string plugin_guid, ...);
void log_output_va_list(LogLevel level, const std::string& message, const std::string& plugin_guid, va_list args);

#define JLOGGERNAME "JAPILauncher"

#define JFATAL(message, ...) log_output_with_fmt(LOG_LEVEL_FATAL, message, JLOGGERNAME, ##__VA_ARGS__);
#define JERROR(message, ...) log_output_with_fmt(LOG_LEVEL_ERROR, message, JLOGGERNAME, ##__VA_ARGS__);
#define JWARN(message, ...) log_output_with_fmt(LOG_LEVEL_WARN, message, JLOGGERNAME, ##__VA_ARGS__);
#define JINFO(message, ...) log_output_with_fmt(LOG_LEVEL_INFO, message, JLOGGERNAME, ##__VA_ARGS__);
#define JDEBUG(message, ...) log_output_with_fmt(LOG_LEVEL_DEBUG, message, JLOGGERNAME, ##__VA_ARGS__);
#define JTRACE(message, ...) log_output_with_fmt(LOG_LEVEL_TRACE, message, JLOGGERNAME, ##__VA_ARGS__);

#endif //JAPI_PRELOAD_LOGGER_H