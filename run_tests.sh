#!/bin/bash

if [ ! -d "build" ]; then
    echo "Build directory does not exist."
    echo "Making build directory..."
    mkdir build
    echo "Build directory created."
    echo "Configuring build..."
    cmake -B build
    echo "Build configuration completed."
fi

echo "Building tests..."
cmake --build build
echo "Build completed."

tests=("build/array_test")

echo "Running tests..."
for test in "${tests[@]}"; do
    echo "Running $test with valgrind..."
    valgrind --tool=memcheck \
             --leak-check=full \
             --show-leak-kinds=all \
             --track-origins=yes \
             --undef-value-errors=yes \
             --error-exitcode=1 \
             $test
done
echo "All tests completed."
