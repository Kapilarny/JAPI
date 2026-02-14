project(JDOWNLOAD CXX)

# Build as static library
add_library(JDOWNLOAD STATIC
        src/downloader.cpp
        src/downloader.h
)

# Add include directories
target_include_directories(JDOWNLOAD PUBLIC src)

target_link_libraries(JDOWNLOAD pthread winhttp)