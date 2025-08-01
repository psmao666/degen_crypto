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

# Add Quill library
FetchContent_Declare(
    quill
    GIT_REPOSITORY https://github.com/odygrd/quill.git
    GIT_TAG v10.0.1
)
FetchContent_MakeAvailable(quill)

# Add options for exchanges
option(BINANCE "Build with Binance exchange" OFF)
option(BYBIT "Build with Bybit exchange" OFF)

if(BINANCE)
    message(STATUS "Building for Binance exchange")
elseif(BYBIT)
    message(STATUS "Building for Bybit exchange")
else()
    message(FATAL_ERROR "No exchange defined. Please configure with -DBINANCE=ON or -DBYBIT=ON.")
endif()

# Set output directories based on build type and exchange
if(BINANCE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/build/binance/debug)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/build/binance/release)
elseif(BYBIT)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/build/bybit/debug)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/build/bybit/release)
endif()

# Add source files
set(SOURCES
    main.cpp
    common/utils/logger.cpp
    exchange/orderbook.cpp
    kitchen/strategy_manager.cpp
)

if(BINANCE)
    list(APPEND SOURCES exchange/binance/binance.cpp)
elseif(BYBIT)
    list(APPEND SOURCES exchange/bybit/bybit.cpp)
endif()

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

if(DEBUG)
    # Add compiler flags for debugging
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wno-deprecated-declarations
        -g                    # Generate debug iSnformation
        -O0                   # Disable optimization for better debugging
        -fno-omit-frame-pointer  # Keep frame pointers for better stack traces
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE 
    -O3                        # Maximum optimization
    -march=native              # Optimize for the host CPU
    -flto                      # Link Time Optimization
    -DNDEBUG                   # Disable assert()
    )
endif()

# Handle DEBUG definition
if(DEBUG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
endif() 

# ADD EXCHANGE def

if(BINANCE)    
    target_compile_options(${PROJECT_NAME} PRIVATE
        -DBINANCE
    )
elseif(BYBIT)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -DBYBIT
    )
endif()