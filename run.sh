#!/bin/bash

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Go to build directory
cd build

# Run cmake, make, and execute the program
echo "Running cmake..."
cmake .. && echo "Building project..." && make && echo "Starting application..." && ./cpp_rest

# Return to root directory
cd ..
