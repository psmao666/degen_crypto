cmake_minimum_required(VERSION 3.10)

# Set project name and language
project(degen_crypto CXX)

# Enable compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard to C++20 (more widely supported than C++23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Add fmt library
include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.2.1  # Using a stable version
)
FetchContent_MakeAvailable(fmt)

# Add json library
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/release)

# Add source files
set(SOURCES
    main.cpp
    exchange/binance/binance.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries and add include paths
target_link_libraries(${PROJECT_NAME} PRIVATE 
    fmt::fmt
    nlohmann_json::nlohmann_json
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
) 