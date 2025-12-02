//
// Created by kapil on 2.12.2025.
//

#include "events.h"

#include "logger.h"

void event_manager::register_event_listener(const std::string &event_name, JAPI_EventListener listener) {
    event_listeners[event_name].push_back(listener);
}

void event_manager::register_cancellable_event_listener(const std::string &event_name,
    JAPI_CancellableEventListener listener) {
    cancellable_event_listeners[event_name].push_back(listener);
}

void event_manager::transmit_event(const std::string &event_name, void *event_data) {
    auto it = event_listeners.find(event_name);

    if (it == event_listeners.end()) {
        JWARN("Trying to transmit event '%s' with no listeners!", event_name.c_str());
        return;
    }

    for (const auto& listener : it->second) {
        listener(event_data);
    }
}

bool event_manager::transmit_event_cancellable(const std::string &event_name, void *event_data) {
    auto it = cancellable_event_listeners.find(event_name);
    if (it == cancellable_event_listeners.end()) {
        JWARN("Trying to transmit cancellable event '%s' with no listeners!", event_name.c_str());
        return false;
    }

    bool should_cancel = false;
    for (const auto& listener : it->second) {
        should_cancel |= listener(event_data);
    }

    return should_cancel;
}

void event_manager::init() {
    if (instance != nullptr) {
        JERROR("Trying to reinitialize event_manager!");
        return;
    }

    instance = new event_manager();
}

event_manager& event_manager::get() {
    if (instance == nullptr) {
        JFATAL("Trying to access event_manager before initialization!");
    }

    return *instance;
}
