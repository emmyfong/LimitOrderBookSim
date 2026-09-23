# LimitOrderBookSim

A C++17 limit order book matching engine. Orders are read from a CSV file, streamed through a
producer/consumer thread pipeline, and matched against a live book using price-time priority.

## Features

- Price-time priority matching for LIMIT orders, immediate fill-or-skip for MARKET orders
- Partial fills, order cancellation, and best bid/ask queries
- Multithreaded pipeline: a producer thread parses the CSV while a consumer thread feeds the
  matching engine through a thread-safe queue
- Unit tests covering matching, partial fills, and cancellation on both sides of the book

## Build & Run

```bash
./build.sh   # compiles and runs the simulator against data/orders.csv
./test.sh    # compiles and runs the unit test suite
```

## Project Structure

```
include/    Order, OrderBook, Simulator, ThreadSafeQueue, CSVParser headers
src/        implementations + main.cpp entry point
tests/      unit tests (asserts, no framework)
data/       sample CSV order data
```

## Order CSV Format

```
id,side,type,price,quantity,timestamp
1,BUY,LIMIT,15000,100,1
```

## Status

Actively being worked on — see commit history for the matching engine, threading pipeline, and CI setup.
