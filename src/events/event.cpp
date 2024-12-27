#include "event.h"

void event_transmitter::init() {
    instance = std::make_unique<event_transmitter>();
}

void event_transmitter::transmit_event(const std::string &eventName, void* data) {
    for (auto& callback : instance->cpp_event_callbacks[eventName]) {
        callback(data);
    }
}

bool event_transmitter::transmit_event_cancellable(const std::string &eventName, void *data) {
    bool cancelled = false;

    for (auto& callback : instance->cpp_event_callbacks[eventName]) {
        auto result = callback(data);

        if(!result) cancelled = true;
    }
    return cancelled;
}

void event_transmitter::register_callback(const std::string & eventName, EventCallback callback) {
    instance->cpp_event_callbacks[eventName].push_back(callback);
}