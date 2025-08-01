#!/bin/bash

# Print colorful status messages
print_status() {
    echo -e "\033[1;34m[*] $1\033[0m"
}

# Error handling
set -e

# Parse command line arguments
DEBUG_BUILD=false
EXCHANGE="binance"
EXCHANGE_CMAKE_FLAG="-DBINANCE=ON"
for ((i=1; i<=$#; i++)); do
    arg=${!i}
    case $arg in
        --debug)
            DEBUG_BUILD=true
            ;;
        --mock)
            MOCK_TRADE=true
            ;;
        -e)
            next_i=$((i+1))
            next_arg=${!next_i}
            if [ "$next_arg" = "binance" ]; then
                EXCHANGE="binance"
                EXCHANGE_CMAKE_FLAG="-DBINANCE=ON"
            elif [ "$next_arg" = "bybit" ]; then
                EXCHANGE="bybit"
                EXCHANGE_CMAKE_FLAG="-DBYBIT=ON"
            else
                echo "Unknown exchange: $next_arg"
                exit 1
            fi
            i=$next_i
            ;;
    esac
done

print_status "Starting <Degen Crypto> build process for exchange: $EXCHANGE ..."

# Determine build directory based on debug or release and exchange
if [ "$DEBUG_BUILD" = true ]; then
    BUILD_DIR="build/$EXCHANGE/debug"
else
    BUILD_DIR="build/$EXCHANGE/release"
fi

print_status "Creating build directory..."
mkdir -p "$BUILD_DIR"

print_status "Entering build directory..."
cd "$BUILD_DIR"

# Generate build files with CMake
print_status "Generating CMake build files..."
if [ "$DEBUG_BUILD" = true ]; then
    print_status "Building in DEBUG mode with GDB support..."
    if [ "$MOCK_TRADE" = true ]; then
        print_status "Building with MOCK_TRADE enabled..."
        cmake -DCMAKE_BUILD_TYPE=Debug -DDEBUG=ON -DMOCK_TRADE=ON $EXCHANGE_CMAKE_FLAG ../../..
    else
        print_status "Building with MOCK_TRADE disabled..."
        cmake -DCMAKE_BUILD_TYPE=Debug -DDEBUG=ON $EXCHANGE_CMAKE_FLAG ../../..
    fi
else
    print_status "Building in RELEASE mode..."
    if [ "$MOCK_TRADE" = true ]; then
        print_status "Building with MOCK_TRADE enabled..."
        cmake -DCMAKE_BUILD_TYPE=Release -DMOCK_TRADE=ON $EXCHANGE_CMAKE_FLAG ../../..
    else
        print_status "Building with MOCK_TRADE disabled..."
        cmake -DCMAKE_BUILD_TYPE=Release $EXCHANGE_CMAKE_FLAG ../../..
    fi
fi

# Build the project
print_status "Building project..."
cmake --build .

print_status "Build complete! Binary is located in $BUILD_DIR/"

# Return to original directory
cd ../../..

# Make the binary executable
chmod +x "$BUILD_DIR/degen_crypto"

# Set timezone to Hong Kong
export TZ=Asia/Hong_Kong

if [ "$DEBUG_BUILD" = true ]; then
    print_status "Debug build completed! You can now debug with:"
    print_status "  gdb $BUILD_DIR/degen_crypto"
    print_status "  or run with: $BUILD_DIR/degen_crypto"
else
    print_status "Release build completed! You can now run: $BUILD_DIR/degen_crypto"
fi 