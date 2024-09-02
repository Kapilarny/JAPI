#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <sol.hpp>

#include "../exports/JojoAPI.h"

class EventTransmitter {
public:
    static void Init();

    static void TransmitEvent(std::string eventName, void* data);
    static bool TransmitEventCancellable(std::string eventName, void* data);

    static void RegisterCallback(std::string eventName, EventCallback callback);
    static void RegisterLuaCallback(std::string eventName, sol::function callback);
private:
    static inline std::unique_ptr<EventTransmitter> instance;

    std::unordered_map<std::string, std::vector<EventCallback>> cppEventCallbacks;
    std::unordered_map<std::string, std::vector<sol::function>> luaEventCallbacks;
};