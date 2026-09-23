#!/bin/bash
set -e

echo "Compiling Cancel Latency Benchmark (optimized build)..."
g++ -std=c++17 -O3 -Iinclude src/OrderBook.cpp benchmarks/bench_cancel.cpp -o bench_cancel

echo "Running Benchmark..."
./bench_cancel
