project(JAPI_main LANGUAGES C CXX)

add_library(JAPI SHARED
        src/main.cpp
        src/japi.cpp
        src/JoJoAPI.cpp
        src/subsystems/modloader.cpp
        src/subsystems/hook.cpp
        src/subsystems/hook.h
        src/subsystems/kiero/kiero.cpp
        src/subsystems/kiero/kiero.h
        src/subsystems/gui.cpp
        src/subsystems/gui.h
        src/subsystems/kiero/kiero_impl.cpp
        src/subsystems/events.cpp
        src/subsystems/events.h
        src/util/mem.h
)

target_include_directories(JAPI PRIVATE
        src
        exports
)

target_link_libraries(JAPI PUBLIC
        minhook
        imgui_shared
        JLOG
        JConfig
)