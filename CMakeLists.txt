cmake_minimum_required(VERSION 3.15)
project(BigBrother VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src/common)
include_directories(${CMAKE_SOURCE_DIR}/third_party)

# Common library (shared between monitor and viewer)
add_subdirectory(src/common)

# Monitor application
add_subdirectory(src/monitor)

# Viewer application (optional, enable with -DBUILD_VIEWER=ON)
option(BUILD_VIEWER "Build the ImGui viewer application" ON)
if(BUILD_VIEWER)
    add_subdirectory(src/viewer)
endif()

# Print configuration
message(STATUS "BigBrother Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build viewer: ${BUILD_VIEWER}")
