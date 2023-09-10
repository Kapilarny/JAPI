#include "event.h"

void EventTransmitter::TransmitEvent(std::string eventName, void* data) {
    for (auto& callback : instance->cppEventCallbacks[eventName]) {
        callback(data);
    }

    for (auto& callback : instance->luaEventCallbacks[eventName]) {
        callback(data);
    }
}

void EventTransmitter::RegisterCallback(std::string eventName, EventCallback callback) {
    instance->cppEventCallbacks[eventName].push_back(callback);
}

void EventTransmitter::RegisterLuaCallback(std::string eventName, sol::function callback) {
    instance->luaEventCallbacks[eventName].push_back(callback);
}