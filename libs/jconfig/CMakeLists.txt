project(src)

add_library(JConfig STATIC
        src/config.h
        src/config.cpp
)

target_include_directories(JConfig PUBLIC
    ${CMAKE_SOURCE_DIR}/external/toml
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)