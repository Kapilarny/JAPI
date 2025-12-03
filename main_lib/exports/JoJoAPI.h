//
// Created by kapil on 1.12.2025.
//

#ifndef JAPI_PRELOAD_JOJOAPI_H
#define JAPI_PRELOAD_JOJOAPI_H

#ifndef JEXP
#define JEXP extern "C" __declspec(dllexport)
#endif

// Keep the header C compatible
#include <stdint.h> // NOLINT(*-deprecated-headers)

typedef struct JAPIModMeta {
    const char* name;
    const char* author;
    const char* guid;
    const char* version;
    const char* description;
} JAPIModMeta;

typedef uint64_t JAPIHookHandle;
typedef struct JAPIHookMeta {
    uint64_t target;
    void* detour;
    void* original;
    const char* name;

    bool game_function;
} JAPIHookMeta;

typedef struct JAPIString {
    const char* data;
    uint32_t length; // does not include the null terminator
} JAPIString;

typedef enum JAPI_LOG_LEVEL {
    JAPI_LOG_LEVEL_FATAL = 0,
    JAPI_LOG_LEVEL_ERROR = 1,
    JAPI_LOG_LEVEL_WARN = 2,
    JAPI_LOG_LEVEL_INFO = 3,
    JAPI_LOG_LEVEL_DEBUG = 4,
    JAPI_LOG_LEVEL_TRACE = 5
} JAPI_LOG_LEVEL;

typedef void(*JAPI_EventListener)(void* event_data);
typedef bool(*JAPI_CancellableEventListener)(void* event_data);

#pragma region Memory Functions

/**
 * Gets the base address of the game's main module.
 * @return Base address of the game's main module.
 */
JEXP uint64_t JAPI_GetModuleBaseAddress();

/**
 * Searches for a signature in the game's memory.
 * @param signature Signature string to search for (e.g. "48 ?? ?? 0A").
 * @return Memory address corresponding to the given signature, or 0 if not found.
 */
JEXP uint64_t JAPI_FindSignature(const char* signature);

/**
 * Searches for a string in the game's memory.
 * @param str String to search for. (e.g. "Hello, World!")
 * @return Memory address corresponding to the given string, or 0 if not found.
 */
JEXP uint64_t JAPI_FindString(const char* str);

#pragma endregion

#pragma region Hook Functions

///
/// Hook Functions
///

/**
 * Registers a hook with the given metadata.
 * @return A handle to the registered hook, or 0 on failure.
 */
JEXP JAPIHookHandle JAPI_RegisterHook(JAPIHookMeta hook_meta);

/**
 * Unregisters a hook with the given handle.
 * @warning Unregistering a foreign hook is considered illegal.
 * @return 1 on success, 0 on failure.
 */
JEXP bool JAPI_UnregisterHook(JAPIHookHandle hook_handle);

#pragma endregion

#pragma region Config Functions

/**
 * Binds a string config value to the given path.
 * @param key Config key to bind to. (e.g. "graphics_resolution")
 * @param default_value Default value if the config does not exist.
 * @warning Returned JAPIString pointer must be freed using JAPI_FreeString.
 * @return A JAPIString pointer containing the config value, or null if failed
 */
JEXP JAPIString* JAPI_ConfigBindString(const char* key, const char* default_value);

/**
 * Binds an integer config value to the given path.
 * @param key Config key to bind to. (e.g. "max_framerate")
 * @param default_value Default value if the config does not exist.
 * @return The integer config value.
 */
JEXP int JAPI_ConfigBindInt(const char* key, int default_value);

/**
 * Binds a boolean config value to the given path.
 * @param key Config key to bind to. (e.g. "fullscreen_enabled")
 * @param default_value Default value if the config does not exist.
 * @return The boolean config value.
 */
JEXP bool JAPI_ConfigBindBool(const char* key, bool default_value);

/**
 * Binds a float config value to the given path.
 * @param key Config key to bind to. (e.g. "mouse_sensitivity")
 * @param default_value Default value if the config does not exist.
 * @return The float config value.
 */
JEXP float JAPI_ConfigBindFloat(const char* key, float default_value);

/**
 * Sets a string config value at the given path.
 * @param key Config key to set. (e.g. "graphics_resolution")
 * @param value Value to set.
 */
JEXP void JAPI_ConfigSetString(const char* key, const char* value);

/**
 * Sets an integer config value at the given path.
 * @param key Config key to set. (e.g. "max_framerate")
 * @param value Value to set.
 */
JEXP void JAPI_ConfigSetInt(const char* key, int value);

/**
 * Sets a boolean config value at the given path.
 * @param key Config key to set. (e.g. "fullscreen_enabled")
 * @param value Value to set.
 */
JEXP void JAPI_ConfigSetBool(const char* key, bool value);

/**
 * Sets a float config value at the given path.
 * @param key Config key to set. (e.g. "mouse_sensitivity")
 * @param value Value to set.
 */
JEXP void JAPI_ConfigSetFloat(const char* key, float value);

#pragma endregion

#pragma region Event Functions

/**
 * Registers an event listener for the given event name.
 * @param event_name Name of the event to listen to.
 * @param listener Function pointer to the event listener.
 */
JEXP void JAPI_RegisterEventListener(const char* event_name, JAPI_EventListener listener);

/**
 * Registers a cancellable event listener for the given event name.
 * @param event_name Name of the event to listen to.
 * @param listener Function pointer to the cancellable event listener.
 */
JEXP void JAPI_RegisterCancellableEventListener(const char* event_name, JAPI_CancellableEventListener listener);

/**
 * Transmits an event with the given name and data.
 * @param event_name Name of the event to transmit.
 * @param event_data Pointer to the event data.
 */
JEXP void JAPI_TransmitEvent(const char* event_name, void* event_data);

/**
 * Transmits a cancellable event with the given name and data.
 * @param event_name Name of the event to transmit.
 * @param event_data Pointer to the event data.
 * @param cancelled Pointer to a boolean that will be set to true if the event was cancelled
 */
JEXP void JAPI_TransmitEventCancellable(const char* event_name, void* event_data, bool* cancelled);

#pragma endregion

#pragma region Utility Functions

/**
 * Gets the directory reserved for the plugin.
 * Returned directory is guaranteed to exist.
 * @return A JAPIString pointer containing the reserved directory path.
 * @warning Returned JAPIString pointer must be freed using JAPI_FreeString.
 */
JEXP JAPIString* JAPI_GetPluginReservedDirectory();

/**
 * Frees a JAPIString allocated by JoJoAPI.
 */
JEXP void JAPI_FreeString(JAPIString* japi_string);

/**
 * Logs a message with the given log level.
 * Plugins should prefer using this function for logging.
 */
JEXP void JAPI_LogMessage(JAPI_LOG_LEVEL level, const char* message, ...);

#pragma endregion

#endif //JAPI_PRELOAD_JOJOAPI_H