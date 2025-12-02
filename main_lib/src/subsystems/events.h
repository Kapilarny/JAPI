//
// Created by kapil on 2.12.2025.
//

#ifndef JAPI_PRELOAD_EVENTS_H
#define JAPI_PRELOAD_EVENTS_H
#include <string>
#include <unordered_map>
#include <vector>

#include "JoJoAPI.h"

class event_manager {
public:
    static void init();

    static event_manager& get();

    void register_event_listener(const std::string& event_name, JAPI_EventListener listener);
    void register_cancellable_event_listener(const std::string& event_name, JAPI_CancellableEventListener listener);

    void transmit_event(const std::string& event_name, void* event_data);
    bool transmit_event_cancellable(const std::string& event_name, void* event_data);
private:
    event_manager() = default;

    static inline event_manager* instance = nullptr;

    std::unordered_map<std::string, std::vector<JAPI_EventListener>> event_listeners;
    std::unordered_map<std::string, std::vector<JAPI_CancellableEventListener>> cancellable_event_listeners;
};

#endif //JAPI_PRELOAD_EVENTS_H