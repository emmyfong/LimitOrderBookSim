#!/bin/bash

# Exit if command exits with non 0 status
set -e

echo "Compiling Limit Order Book Simulator..."
g++ -std=c++17 -O3 -Iinclude src/*.cpp -o lob_sim

echo "Build successful"