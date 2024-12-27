//
// Created by user on 27.12.2024.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <stdarg.h>
#include <memory>

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

typedef enum LogLevel {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} LogLevel;

void LogOutputWithFmt(LogLevel level, const std::string& message, const std::string& pluginGUID, ...);
void LogOutputVaList(LogLevel level, const std::string& message, const std::string& pluginGUID, va_list args);

#define JFATAL(message, ...) LogOutputWithFmt(LOG_LEVEL_FATAL, message, "JoJoAPI", ##__VA_ARGS__);
#define JERROR(message, ...) LogOutputWithFmt(LOG_LEVEL_ERROR, message, "JoJoAPI", ##__VA_ARGS__);
#define JWARN(message, ...) LogOutputWithFmt(LOG_LEVEL_WARN, message, "JoJoAPI", ##__VA_ARGS__);
#define JINFO(message, ...) LogOutputWithFmt(LOG_LEVEL_INFO, message, "JoJoAPI", ##__VA_ARGS__);
#define JDEBUG(message, ...) LogOutputWithFmt(LOG_LEVEL_DEBUG, message, "JoJoAPI", ##__VA_ARGS__);
#define JTRACE(message, ...) LogOutputWithFmt(LOG_LEVEL_TRACE, message, "JoJoAPI", ##__VA_ARGS__);

#define LOG_FATAL(pluginGUID, message, ...) LogOutputWithFmt(LOG_LEVEL_FATAL, message, pluginGUID, ##__VA_ARGS__);

#define LOG_ERROR(pluginGUID, message, ...) LogOutputWithFmt(LOG_LEVEL_ERROR, message, pluginGUID, ##__VA_ARGS__);

#if LOG_WARN_ENABLED == 1
// Logs a warning-level message
#define LOG_WARN(pluginGUID, message, ...) LogOutputWithFmt(LOG_LEVEL_WARN, message, pluginGUID, ##__VA_ARGS__);
#else
// Does nothing when LOG_WARN_ENABLED != 1
#define LOG_WARN(pluginGUID, message, ...)
#endif

#if LOG_INFO_ENABLED == 1
// Logs a info-level message
#define LOG_INFO(pluginGUID, message, ...) LogOutputWithFmt(LOG_LEVEL_INFO, message, pluginGUID, ##__VA_ARGS__);
#else
// Does nothing when LOG_INFO_ENABLED != 1
#define LOG_INFO(pluginGUID, message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
// Logs a info-level message
#define LOG_DEBUG(pluginGUID, message, ...) LogOutputWithFmt(LOG_LEVEL_DEBUG, message, pluginGUID, ##__VA_ARGS__);
#else
// Does nothing when LOG_DEBUG_ENABLED != 1
#define LOG_DEBUG(pluginGUID, message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
// Logs a info-level message
#define LOG_TRACE(pluginGUID, message, ...) LogOutputWithFmt(LOG_LEVEL_TRACE, message, pluginGUID, ##__VA_ARGS__);
#else
// Does nothing when LOG_TRACE_ENABLED != 1
#define LOG_TRACE(pluginGUID, message, ...)
#endif

#endif //LOGGER_H
