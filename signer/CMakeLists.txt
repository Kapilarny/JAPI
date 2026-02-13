project(JAPISigner CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -static")

add_executable(JAPISigner
        src/main.cpp
        src/logger.cpp
        src/logger.h
        src/crypt.cpp
        src/crypt.h
        src/binary_file.cpp
        src/binary_file.h)

set(sodium "${CMAKE_SOURCE_DIR}/external/libsodium/lib/libsodium.a")

target_include_directories(JAPISigner PRIVATE src ../external/libsodium/include ../external/internal/)
target_link_libraries(JAPISigner ${sodium} pthread winhttp)