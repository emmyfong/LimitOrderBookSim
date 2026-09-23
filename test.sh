#!/bin/bash
set -e

echo "Compiling Test Suite..."

g++ -std=c++17 -Iinclude src/OrderBook.cpp tests/test_orderbook.cpp -o run_tests

echo "Running Tests..."
./run_tests