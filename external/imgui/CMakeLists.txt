cmake_minimum_required(VERSION 3.0)
project(imgui_shared VERSION 1.0.0 LANGUAGES CXX)

add_library(imgui_shared SHARED
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    imgui_impl_dx11.cpp
    imgui_impl_win32.cpp
)

target_include_directories(imgui_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(imgui_shared PUBLIC
        d3d11
        dwmapi
        D3DCompiler)