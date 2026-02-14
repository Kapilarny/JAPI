project(JSIGN CXX)

# Build as static library
add_library(JSIGN STATIC
        src/crypt.cpp
        src/crypt.h
        src/binary_file.cpp
        src/binary_file.h)

# Add include directories
target_include_directories(JSIGN PUBLIC src external/libsodium/include internal/)

# Link against libsodium
set(sodium "${PROJECT_SOURCE_DIR}/external/libsodium/lib/libsodium.a")
target_link_libraries(JSIGN ${sodium} pthread)