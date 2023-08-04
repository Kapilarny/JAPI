cmake_minimum_required(VERSION 3.16)
project(JAPI C CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_STATIC_LIBRARY_PREFIX "")

add_subdirectory(external/minhook)

file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp" "src/*.h" "src/*.hpp")

add_link_options(-static -static-libgcc -static-libstdc++)

add_library(d3dcompiler_47 SHARED ${SOURCES} "dll.def")

target_include_directories(d3dcompiler_47 PRIVATE "src" "external/minhook/include")
target_link_libraries(d3dcompiler_47 PRIVATE minhook)