#include "Simulator.h"
#include "CSVParser.h"
#include <algorithm> //for std::sort
#include <chrono>
#include <iostream>
#include <vector>

namespace {

double percentile(const std::vector<double>& sortedMicros, double p) {
    if (sortedMicros.empty()) {
        return 0.0;
    }
    std::size_t index = static_cast<std::size_t>(p * static_cast<double>(sortedMicros.size() - 1));
    return sortedMicros[index];
}

} // namespace

Simulator::Simulator(const std::string& filename) : filename_(filename) {}

void Simulator::producerLoop() {
    //parse file and then push to queue
    std::vector<Order> orders = CSVParser::parse(filename_);

    for (const auto& order : orders) {
        orderQueue_.push(order);
    }

    //tell consumer file ready
    orderQueue_.setFinished();
}

void Simulator::consumerLoop() {
    Order order;

    //pop orders out of queue until return false
    while (orderQueue_.pop(order)) {
        auto start = std::chrono::high_resolution_clock::now();
        OrderResult result = lob_.addOrder(order);
        auto end = std::chrono::high_resolution_clock::now();

        latencyMicros_.push_back(std::chrono::duration<double, std::micro>(end - start).count());

        bool remainderDidNotRest = result.status == OrderStatus::Rejected ||
            (result.status == OrderStatus::PartiallyFilled && result.reason != nullptr);
        if (remainderDidNotRest) {
            rejections_.emplace_back(order.orderId, result.reason);
        }
    }
}

void Simulator::printSummary(double totalElapsedMs) const {
    const std::vector<Trade>& trades = lob_.getTrades();

    uint64_t totalVolume = 0;
    for (const auto& trade : trades) {
        totalVolume += trade.quantity;
    }

    std::vector<double> sortedLatency = latencyMicros_;
    std::sort(sortedLatency.begin(), sortedLatency.end());

    std::cout << "[*] Simulation Complete\n";
    std::cout << "Orders Processed: " << latencyMicros_.size() << "\n";
    std::cout << "Trades Executed: " << trades.size() << " (" << totalVolume << " shares)\n";
    std::cout << "Total Time Elapsed: " << totalElapsedMs << " ms\n";

    if (!sortedLatency.empty()) {
        double throughputPerSec = static_cast<double>(latencyMicros_.size()) / (totalElapsedMs / 1000.0);
        std::cout << "Throughput: " << throughputPerSec << " orders/sec\n";
        std::cout << "addOrder() Latency (us) - p50: " << percentile(sortedLatency, 0.50)
                   << "  p99: " << percentile(sortedLatency, 0.99)
                   << "  p99.9: " << percentile(sortedLatency, 0.999)
                   << "  max: " << sortedLatency.back() << "\n";
    }

    if (!rejections_.empty()) {
        std::cout << "Rejections: " << rejections_.size() << "\n";
        for (const auto& [orderId, reason] : rejections_) {
            std::cout << "  - order " << orderId << ": " << reason << "\n";
        }
    }
}

void Simulator::run() {
    std::cout << "[*] Starting Multithreaded Engine...\n";
    auto start = std::chrono::high_resolution_clock::now();

    //launch threads
    std::thread producerThread(&Simulator::producerLoop, this);
    std::thread consumerThread(&Simulator::consumerLoop, this);

    //wait for both threads to finish before exiting
    producerThread.join();
    consumerThread.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    printSummary(elapsed.count());
}
