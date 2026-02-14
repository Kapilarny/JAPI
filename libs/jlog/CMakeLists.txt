project(JLOG CXX)

# Build as static library
add_library(JLOG STATIC
        src/logger.cpp
        src/logger.h
)

# Add include directories
target_include_directories(JLOG PUBLIC src)