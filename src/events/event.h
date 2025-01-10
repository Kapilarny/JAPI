//
// Created by user on 27.12.2024.
//

#ifndef EVENT_H
#define EVENT_H

#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <sol.hpp>

#include "exports/JAPIEvents.h"

class event_transmitter {
public:
    static void init();

    static void transmit_event(const std::string &eventName, void* data);
    static bool transmit_event_cancellable(const std::string &eventName, void* data);

    static void register_callback(const std::string &eventName, EventCallback callback);
private:
    static inline std::unique_ptr<event_transmitter> instance;

    std::unordered_map<std::string, std::vector<EventCallback>> cpp_event_callbacks;
};

#endif //EVENT_H
