#!/bin/bash

# Exit if command exits with non 0 status
set -e

echo "Compiling Limit Order Book Simulator..."
g++ -std=c++17 -O3 -pthread -Iinclude src/*.cpp -o lob_sim

echo "Build successful"
# Pass an optional CSV path through, e.g. ./build.sh path/to.csv
./lob_sim "$@"
