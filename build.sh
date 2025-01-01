#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Function to print messages
function echo_msg() {
    echo "===================================="
    echo "$1"
    echo "===================================="
}

# Define project and build paths
PROJECT_DIR="/home/ziaad/code/pokerBot_new"
BUILD_DIR="$PROJECT_DIR/build"
CLANG_MODULE_CACHE="$HOME/.cache/clang/ModuleCache"
CLANGD_CACHE="$HOME/.cache/clangd/"

# Navigate to the project directory
echo_msg "Navigating to project directory: $PROJECT_DIR"
cd "$PROJECT_DIR"

# Remove the build directory if it exists
if [ -d "$BUILD_DIR" ]; then
    echo_msg "Removing existing build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
else
    echo_msg "Build directory does not exist. Skipping removal."
fi

# Remove Clang module caches
if [ -d "$CLANG_MODULE_CACHE" ]; then
    echo_msg "Clearing Clang module cache: $CLANG_MODULE_CACHE"
    rm -rf "$CLANG_MODULE_CACHE"
else
    echo_msg "Clang module cache directory does not exist. Skipping removal."
fi

if [ -d "$CLANGD_CACHE" ]; then
    echo_msg "Clearing Clangd cache: $CLANGD_CACHE"
    rm -rf "$CLANGD_CACHE"
else
    echo_msg "Clangd cache directory does not exist. Skipping removal."
fi

# Create a fresh build directory
echo_msg "Creating build directory: $BUILD_DIR"
mkdir "$BUILD_DIR"

# Navigate into the build directory
echo_msg "Entering build directory: $BUILD_DIR"
cd "$BUILD_DIR"

# Configure the project with CMake using Ninja
echo_msg "Configuring the project with CMake"
cmake -G Ninja \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_CXX_SCAN_FOR_MODULES=ON \
      ..

# Build the project
echo_msg "Building the project with Ninja"
cmake --build .

# Run tests with verbose output
echo_msg "Running tests with CTest"
ctest --verbose

echo_msg "Build and test cycle completed successfully!"
