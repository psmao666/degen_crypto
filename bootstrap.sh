#!/bin/bash

# Print colorful status messages
print_status() {
    echo -e "\033[1;34m[*] $1\033[0m"
}

# Error handling
set -e

print_status "Starting <Degen Crypto> build process..."

# Create build directory if it doesn't exist
print_status "Creating build directory..."
mkdir -p build

# Navigate to build directory
print_status "Entering build directory..."
cd build

# Generate build files with CMake
print_status "Generating CMake build files..."
cmake ..

# Build the project
print_status "Building project..."
cmake --build .

print_status "Build complete! Binary is located in build/release/"

# Return to original directory
cd ..

# Make the binary executable
chmod +x build/release/degen_crypto

print_status "You can now run the program with: ./build/release/degen_crypto" 