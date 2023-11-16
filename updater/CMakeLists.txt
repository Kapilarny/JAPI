# Create a basic C++17 executable project
cmake_minimum_required(VERSION 3.10)
project(JAPIUpdater VERSION 1.0.0 LANGUAGES CXX)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)

# Add the source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add the executable
add_executable(JAPIUpdater ${SOURCES})
target_include_directories(JAPIUpdater PRIVATE "../external/toml")

# add winhttp
target_link_libraries(JAPIUpdater PRIVATE winhttp)