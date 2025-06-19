cmake_minimum_required(VERSION 3.10)

# Set project name and language
project(degen_crypto CXX)

# Enable parallel compilation
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
endif()

# Enable compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard to C++20 (more widely supported than C++23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add Boost from GitHub
include(FetchContent)
FetchContent_Declare(
    boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.84.0
)
FetchContent_MakeAvailable(boost)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Add fmt library
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

# Add Quil library
FetchContent_Declare(
    quill
    GIT_REPOSITORY https://github.com/odygrd/quill.git
    GIT_TAG v10.0.0
)
FetchContent_MakeAvailable(quill)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/release)

# Add source files
set(SOURCES
    main.cpp
    exchange/binance/binance.cpp
    common/utils/logger.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries and add include paths
target_link_libraries(${PROJECT_NAME} PRIVATE 
    fmt::fmt
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    quill::quill
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${boost_SOURCE_DIR}
)

# Add ignore deprecation flag
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-declarations)

# Handle DEBUG definition
if(DEBUG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
endif() 