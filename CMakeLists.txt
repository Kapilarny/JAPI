cmake_minimum_required(VERSION 3.10)

project(JAPI LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/minhook)

add_subdirectory(libs)
add_subdirectory(applications)
