#pragma once

#include <memory>
#include <string>

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

void SetShouldLogToConsole(bool shouldLog);
void LogOutput(LogLevel level, const std::string &message,
               const std::string &pluginGUID);

#define JFATAL(message) LogOutput(LOG_LEVEL_FATAL, message, "JAPIUpdater");
#define JERROR(message) LogOutput(LOG_LEVEL_ERROR, message, "JAPIUpdater");
#define JWARN(message) LogOutput(LOG_LEVEL_WARN, message, "JAPIUpdater");
#define JINFO(message) LogOutput(LOG_LEVEL_INFO, message, "JAPIUpdater");
#define JDEBUG(message) LogOutput(LOG_LEVEL_DEBUG, message, "JAPIUpdater");
#define JTRACE(message) LogOutput(LOG_LEVEL_TRACE, message, "JAPIUpdater");

#define LOG_FATAL(pluginGUID, message)                                         \
  LogOutput(LOG_LEVEL_FATAL, message, pluginGUID);

#define LOG_ERROR(pluginGUID, message)                                         \
  LogOutput(LOG_LEVEL_ERROR, message, pluginGUID);

#if LOG_WARN_ENABLED == 1
// Logs a warning-level message
#define LOG_WARN(pluginGUID, message)                                          \
  LogOutput(LOG_LEVEL_WARN, message, pluginGUID);
#else
// Does nothing when LOG_WARN_ENABLED != 1
#define LOG_WARN(pluginGUID, message)
#endif

#if LOG_INFO_ENABLED == 1
// Logs a info-level message
#define LOG_INFO(pluginGUID, message)                                          \
  LogOutput(LOG_LEVEL_INFO, message, pluginGUID);
#else
// Does nothing when LOG_INFO_ENABLED != 1
#define LOG_INFO(pluginGUID, message)
#endif

#if LOG_DEBUG_ENABLED == 1
// Logs a info-level message
#define LOG_DEBUG(pluginGUID, message)                                         \
  LogOutput(LOG_LEVEL_DEBUG, message, pluginGUID);
#else
// Does nothing when LOG_DEBUG_ENABLED != 1
#define LOG_DEBUG(pluginGUID, message)
#endif

#if LOG_TRACE_ENABLED == 1
// Logs a info-level message
#define LOG_TRACE(pluginGUID, message)                                         \
  LogOutput(LOG_LEVEL_TRACE, message, pluginGUID);
#else
// Does nothing when LOG_TRACE_ENABLED != 1
#define LOG_TRACE(pluginGUID, message)
#endif