#!/bin/bash

# Script to build and run the GNOME Terminal Clone project (GTK 4 version)

# Exit on any error
set -e

# Default build type if not specified
BUILD_TYPE=${BUILD_TYPE:-Release}

# Number of CPU cores for parallel build (auto-detect or default to 4)
JOBS=$(nproc 2>/dev/null || echo 4)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Function to print usage
usage() {
    echo "Usage: $0 [--debug] [--clean]"
    echo "  --debug  Build in Debug mode (default is Release)"
    echo "  --clean  Remove build directory before building"
    exit 1
}

# Parse command-line arguments
CLEAN_BUILD=false
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --debug) BUILD_TYPE="Debug"; shift ;;
        --clean) CLEAN_BUILD=true; shift ;;
        -h|--help) usage ;;
        *) echo -e "${RED}Unknown option: $1${NC}"; usage ;;
    esac
done

# Check if we're in the project root (look for CMakeLists.txt)
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: CMakeLists.txt not found. Run this from the project root.${NC}"
    exit 1
fi

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf build
fi

# Create build directory if it doesn’t exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi
cd build

# Configure with CMake
echo "Configuring with CMake (Build Type: $BUILD_TYPE)..."
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..

# Build the project
echo "Building with $JOBS parallel jobs..."
cmake --build . -j"$JOBS"

# Check if the executable exists
EXECUTABLE="./gnome-terminal-clone"
if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}Error: Build failed, executable not found.${NC}"
    exit 1
fi

# Run the program
echo -e "${GREEN}Build successful! Running $EXECUTABLE...${NC}"
"$EXECUTABLE"

# Return to project root
cd ..
