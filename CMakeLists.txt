cmake_minimum_required(VERSION 3.18)
project(JAPI C CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_STATIC_LIBRARY_PREFIX "")

add_subdirectory(external/minhook)
# add_subdirectory(external/asmtk)

add_subdirectory(updater)

file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp" "src/*.h" "src/*.hpp")

add_link_options(-static -static-libgcc -static-libstdc++ -Wno-pointer-arith)

add_library(d3dcompiler_47 SHARED ${SOURCES} "dll.def"
        src/exports/JAPIEvents.h
        src/kiero/kiero.cpp
        src/kiero/kiero.h
        src/kiero/d3d11_impl.cpp
        src/kiero/d3d11_impl.h
        src/kiero/win32_impl.cpp
        src/kiero/win32_impl.h
        src/imgui/imconfig.h
        src/imgui/imgui.cpp
        src/imgui/imgui.h
        src/imgui/imgui_demo.cpp
        src/imgui/imgui_draw.cpp
        src/imgui/imgui_impl_dx11.cpp
        src/imgui/imgui_impl_dx11.h
        src/imgui/imgui_impl_win32.cpp
        src/imgui/imgui_impl_win32.h
        src/imgui/imgui_internal.h
        src/imgui/imgui_tables.cpp
        src/imgui/imgui_widgets.cpp
        src/imgui/imstb_rectpack.h
        src/imgui/imstb_textedit.h
        src/imgui/imstb_truetype.h
)

target_include_directories(d3dcompiler_47 PRIVATE "src" "external/minhook/include" "external/lua/include" "external/toml" "external/sol2/" "external/nlohmann")
target_link_libraries(d3dcompiler_47 PRIVATE
        -static
        winpthread
        minhook
        "${CMAKE_CURRENT_SOURCE_DIR}/external/lua/bin/lua54.dll"
        # "${CMAKE_CURRENT_SOURCE_DIR}/external/built/asmjit.dll"
        # "${CMAKE_CURRENT_SOURCE_DIR}/external/built/asmtk.dll"
        d3d11
        dwmapi
        D3DCompiler
)