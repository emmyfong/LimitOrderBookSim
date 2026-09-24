#pragma once
#include "OrderBook.h"
#include "ThreadSafeQueue.h"
#include <string>
#include <thread>
#include <utility>
#include <vector>

class Simulator {
public:
    Simulator(const std::string& filename);
    void run();

private:
    void producerLoop();
    void consumerLoop();
    void printSummary(double totalElapsedMs) const;

    std::string filename_;
    OrderBook lob_;
    ThreadSafeQueue<Order> orderQueue_;

    //per-order addOrder() latency in microseconds
    std::vector<double> latencyMicros_;

    //rejected/stopped orders, reported once at the end of run()
    std::vector<std::pair<uint64_t, const char*>> rejections_;
};
