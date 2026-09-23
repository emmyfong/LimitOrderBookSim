#!/bin/bash
set -e

echo "Compiling OrderBook Test Suite..."
g++ -std=c++17 -Iinclude src/OrderBook.cpp tests/test_orderbook.cpp -o run_tests_orderbook

echo "Running OrderBook Tests..."
./run_tests_orderbook

echo "Compiling CSVParser Test Suite..."
g++ -std=c++17 -Iinclude src/CSVParser.cpp tests/test_csvparser.cpp -o run_tests_csvparser

echo "Running CSVParser Tests..."
./run_tests_csvparser