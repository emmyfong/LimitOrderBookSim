//benchmark: cancelOrder's hash-indexed lookup vs the original O(n) scan
#include "OrderBook.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <vector>

namespace {

constexpr int kTrialsPerSize = 7;
constexpr uint64_t kBasePrice = 10000;

//mirrors the original scan-based cancelOrder, kept here only for comparison
class NaiveBidBook {
public:
    void addOrder(const Order& order) { bids_[order.price].push_back(order); }

    void cancelOrder(uint64_t orderId) {
        for (auto it = bids_.begin(); it != bids_.end(); ++it) {
            auto& queue = it->second;
            for (auto orderIt = queue.begin(); orderIt != queue.end(); ++orderIt) {
                if (orderIt->orderId == orderId) {
                    queue.erase(orderIt);
                    if (queue.empty()) {
                        bids_.erase(it);
                    }
                    return;
                }
            }
        }
    }

private:
    std::map<uint64_t, std::list<Order>, std::greater<uint64_t>> bids_;
};

//one order per price level - worst case for a linear scan
Order makeOrder(int i) {
    return {static_cast<uint64_t>(i), static_cast<uint64_t>(i), OrderSide::BUY, OrderType::LIMIT,
            kBasePrice + static_cast<uint64_t>(i), 10, static_cast<uint64_t>(i)};
}

double medianOf(std::vector<double> samples) {
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

double timeNaiveWorstCaseCancel(int n) {
    NaiveBidBook book;
    for (int i = 1; i <= n; ++i) {
        book.addOrder(makeOrder(i));
    }

    auto start = std::chrono::high_resolution_clock::now();
    book.cancelOrder(1);
    auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double, std::micro>(end - start).count();
}

double timeIndexedWorstCaseCancel(int n) {
    OrderBook book;
    for (int i = 1; i <= n; ++i) {
        book.addOrder(makeOrder(i));
    }

    auto start = std::chrono::high_resolution_clock::now();
    book.cancelOrder(1);
    auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double, std::micro>(end - start).count();
}

void runSize(int n) {
    std::vector<double> naiveTrials;
    std::vector<double> indexedTrials;

    for (int trial = 0; trial < kTrialsPerSize; ++trial) {
        naiveTrials.push_back(timeNaiveWorstCaseCancel(n));
        indexedTrials.push_back(timeIndexedWorstCaseCancel(n));
    }

    double naiveUs = medianOf(naiveTrials);
    double indexedUs = medianOf(indexedTrials);

    std::cout << std::left << std::setw(16) << n << std::right << std::setw(16)
               << std::fixed << std::setprecision(2) << naiveUs << std::setw(16)
               << indexedUs << std::setw(12) << (naiveUs / indexedUs) << "x\n";
}

} // namespace

int main() {
    const std::vector<int> sizes = {1000, 5000, 10000, 50000, 100000, 250000, 500000};

    std::cout << "Cancel latency: O(n) linear scan vs O(log n)+O(1) hash-indexed lookup\n";
    std::cout << "Median of " << kTrialsPerSize
               << " trials, each cancelling the single worst-case order for a freshly built book.\n\n";

    std::cout << std::left << std::setw(16) << "Resting Orders" << std::right
               << std::setw(16) << "Naive (us)" << std::setw(16) << "Indexed (us)"
               << std::setw(12) << "Speedup" << "\n";

    for (int n : sizes) {
        runSize(n);
    }

    return 0;
}
