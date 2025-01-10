cmake_minimum_required(VERSION 3.18)
project(JAPI C CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_STATIC_LIBRARY_PREFIX "")

add_subdirectory(external/minhook)
add_subdirectory(external/imgui)
# add_subdirectory(external/asmtk)

add_subdirectory(updater)

file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp" "src/*.h" "src/*.hpp")

add_link_options(-static -static-libgcc -static-libstdc++ -Wno-pointer-arith)

add_library(d3dcompiler_47 SHARED ${SOURCES} "dll.def"
        src/japi.cpp
        src/japi.h
        src/utils/config.cpp
        src/utils/config.h
        src/utils/logger.cpp
        src/utils/logger.h
        src/utils/game_type.h
        src/events/event.cpp
        src/events/event.h
        src/exports/JAPIEvents.h
        src/kiero/d3d11_impl.cpp
        src/kiero/d3d11_impl.h
        src/kiero/kiero.cpp
        src/kiero/kiero.h
        src/kiero/win32_impl.cpp
        src/kiero/win32_impl.h
        src/mods/mod_manager.cpp
        src/mods/mod_manager.h
        src/exports/JoJoAPI.h
        src/exports/JoJoAPI.cpp
        src/utils/hooks.cpp
        src/utils/hooks.h
        src/utils/mem.h
        src/utils/downloader.cpp
        src/utils/downloader.h
        src/utils/reloader.cpp
        src/utils/reloader.h
)

target_include_directories(d3dcompiler_47 PUBLIC "src" "external/minhook/include" "external/lua/include" "external/toml" "external/sol2/" "external/nlohmann")
target_link_libraries(d3dcompiler_47 PUBLIC
        -static
        winpthread
        minhook
#        "${CMAKE_CURRENT_SOURCE_DIR}/external/lua/bin/lua54.dll"
        # "${CMAKE_CURRENT_SOURCE_DIR}/external/built/asmjit.dll"
        # "${CMAKE_CURRENT_SOURCE_DIR}/external/built/asmtk.dll"
        imgui_shared
        winhttp
)