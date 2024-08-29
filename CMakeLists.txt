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

add_library(d3dcompiler_47 SHARED ${SOURCES} "dll.def")

target_include_directories(d3dcompiler_47 PRIVATE "src" "external/minhook/include" "external/lua/include" "external/toml" "external/sol2/" "external/nlohmann")
target_link_libraries(d3dcompiler_47 PRIVATE
    -static
    winpthread
    minhook 
    "${CMAKE_CURRENT_SOURCE_DIR}/external/lua/bin/lua54.dll"
    # "${CMAKE_CURRENT_SOURCE_DIR}/external/built/asmjit.dll"
    # "${CMAKE_CURRENT_SOURCE_DIR}/external/built/asmtk.dll"
    d3d11
)