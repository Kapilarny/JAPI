cmake_minimum_required(VERSION 3.10)
project(JAPI_preload LANGUAGES C CXX)

# Remove the annoying 'lib' prefix
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# Statically link the C++ standard library (for preload DLL only)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -static")

set(OVERRIDEN_LIB_NAME "JAPIPreload")

add_library(${OVERRIDEN_LIB_NAME} SHARED
        src/dllmain.cpp
)

target_include_directories(${OVERRIDEN_LIB_NAME} PRIVATE
        src
)