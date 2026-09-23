# LimitOrderBookSim

A C++17 limit order book matching engine. Orders are read from a CSV file, streamed through a
producer/consumer thread pipeline, and matched against a live book using price-time priority.

## Features

- Price-time priority matching for LIMIT orders, immediate fill-or-skip for MARKET orders
- Partial fills, order cancellation, and best bid/ask queries
- Self-trade prevention: a trader's incoming order is rejected outright rather than matched
  against their own resting order (Cancel-Newest)
- Explicit accept/reject results: `addOrder` returns fill status and quantity instead of `void`,
  and `cancelOrder` reports whether there was anything to cancel
- Multithreaded pipeline: a producer thread parses the CSV while a consumer thread feeds the
  matching engine through a thread-safe queue
- Unit tests covering matching, partial fills, cancellation on both sides of the book, and
  self-trade prevention

## Build & Run

```bash
./build.sh      # compiles and runs the simulator against data/orders.csv
./test.sh       # compiles and runs the unit test suite
./benchmark.sh  # compiles (-O3) and runs the cancel-latency benchmark
```

## Project Structure

```
include/     Order, OrderBook, Simulator, ThreadSafeQueue, CSVParser headers
src/         implementations + main.cpp entry point
tests/       unit tests (asserts, no framework)
benchmarks/  performance benchmarks
data/        sample CSV order data
```

## Order CSV Format

```
id,traderId,side,type,price,quantity,timestamp
1,100,BUY,LIMIT,15000,100,1
```

## Self-Trade Prevention

`OrderBook::addOrder` checks, at every attempted match, whether the resting order it's about to
trade against belongs to the same `traderId`. If so, the incoming order is rejected outright
(Cancel-Newest) instead of trading with itself - the resting order is left untouched, and any
quantity the incoming order already filled against *other* traders earlier in the same call still
stands. The sample `data/orders.csv` includes a deliberate demo of this (orders 99 and 100, both
trader 7): order 99 rests a bid at $160.00, and order 100 - a crossing sell from the same trader -
is rejected rather than matched. Running `./build.sh` prints an `ORDER REJECTED` line for it.

`addOrder` now returns an `OrderResult{status, filledQuantity, reason}` instead of `void`, so a
caller (or a strategy built on top of this engine) can tell `Filled`, `PartiallyFilled`,
`Accepted`, and `Rejected` apart - and `cancelOrder` returns `CancelStatus::Cancelled` or
`NotFound` instead of silently no-op'ing on an unknown or already-filled order id.

## Performance

`cancelOrder` originally scanned every price level and every resting order to find the one being
cancelled - O(n) in the number of resting orders. It's now backed by a hash index from order ID to
its exact location in the book, so a cancel is an O(log n) price-level lookup followed by an O(1)
list erase, instead of a full linear scan.

[benchmarks/bench_cancel.cpp](benchmarks/bench_cancel.cpp) reproduces both versions side by side
(the original scan is kept there for comparison, not in the production `OrderBook`) and times
cancelling the single worst-case order for a freshly built book at each size, median of 7 trials:

| Resting Orders | Naive Scan (µs) | Indexed (µs) | Speedup    |
|----------------|-----------------|--------------|------------|
| 1,000          | 4.00            | 0.20         | 20x        |
| 5,000          | 24.70           | 0.40         | 62x        |
| 10,000         | 61.60           | 0.80         | 77x        |
| 50,000         | 659.70          | 2.00         | 330x       |
| 100,000        | 1188.90         | 2.60         | 457x       |
| 250,000        | 3720.00         | 3.70         | 1,005x     |
| 500,000        | 8050.80         | 3.60         | 2,236x     |

The naive scan's latency grows with book size, as expected for O(n). The indexed cancel stays
within a few microseconds across three orders of magnitude of book depth.

## Status

Actively being worked on — see commit history for the matching engine, threading pipeline, and CI setup.
