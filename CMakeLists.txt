cmake_minimum_required(VERSION 3.27)
project(TCE)

# set C++ standard
set(CMAKE_CXX_STANDARD 20)

# set supported configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profile" CACHE STRING "" FORCE)

# set configuration if not specified
IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
message("* Current build type: ${CMAKE_BUILD_TYPE}")

# Add options
option(TRACE_EVAL "Output evaluation trace" OFF)

# Set compiler flags :
# Set common flags
add_compile_options(-Wall -Wcast-qual -fno-exceptions -pedantic -Wextra -Wshadow -Wmissing-declarations -Wunused)

# Set debug flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g -DDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Profile")
    add_compile_options(-g -pg)
    add_link_options(-pg)
endif()

# Set optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Profile")
    add_compile_options(
        -O3                     # High level of optimization
        -march=native           # Optimize for the current architecture
        -flto                   # Enable Link-Time Optimization
        -funroll-loops          # Unroll loops for performance
        -ffast-math             # Allow more aggressive floating-point optimizations
        -finline-functions      # Inline small functions for speed
        -fno-rtti               # Disable C++ RTTI
    )
endif()

# Set release flags
if (CMAKE_BUILD_TYPE STREQUAL "Release")       
    add_compile_options(
        -fomit-frame-pointer    # Omit frame pointers for faster function calls
    )
endif()

# Set user-defined flags
if(TRACE_EVAL)
    add_compile_options(-DTRACE_EVAL)
endif()

# add subdirectories
add_subdirectory(src)

# reset all options to default
unset(TRACE_EVAL CACHE)
