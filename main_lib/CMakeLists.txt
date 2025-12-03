project(JAPI_main LANGUAGES C CXX)

add_library(JAPI SHARED
        src/main.cpp
        src/subsystems/config.cpp
        src/subsystems/logger.cpp
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
        ${CMAKE_SOURCE_DIR}/external/toml
        ${CMAKE_SOURCE_DIR}/external/imgui
)
target_link_libraries(JAPI PUBLIC
        minhook
        ${CMAKE_SOURCE_DIR}/external/bin/imgui_shared.dll
)