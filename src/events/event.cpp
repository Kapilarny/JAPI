#include "event.h"

void EventTransmitter::Init() {
    instance = std::make_unique<EventTransmitter>();
}

void EventTransmitter::TransmitEvent(std::string eventName, void* data) {
    for (auto& callback : instance->cppEventCallbacks[eventName]) {
        callback(data);
    }

    for (auto& callback : instance->luaEventCallbacks[eventName]) {
        callback(data);
    }
}

bool EventTransmitter::TransmitEventCancellable(std::string eventName, void *data) {
    bool cancelled = false;

    for (auto& callback : instance->cppEventCallbacks[eventName]) {
        auto result = callback(data);

        if(!result) cancelled = true;
    }

    for (auto& callback : instance->luaEventCallbacks[eventName]) {
        auto result = callback(data);

        if(!result) cancelled = true;
    }

    return cancelled;
}

void EventTransmitter::RegisterCallback(std::string eventName, EventCallback callback) {
    instance->cppEventCallbacks[eventName].push_back(callback);
}

void EventTransmitter::RegisterLuaCallback(std::string eventName, sol::function callback) {
    instance->luaEventCallbacks[eventName].push_back(callback);
}