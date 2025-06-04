cmake_minimum_required(VERSION 3.10)

# Set project name and language
project(degen_crypto CXX)

# Set C++ standard to C++23 (instead of 26)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add fmt library
add_subdirectory(common/utils/fmt)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/release)

# Add executable
add_executable(${PROJECT_NAME} main.cpp)

# Link fmt library and add include paths
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt) 