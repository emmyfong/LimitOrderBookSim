#include "OrderBook.h"
#include "CSVParser.h"
#include <iostream>
#include <chrono>

//Helper vars
uint64_t currentTimestep = 1;
uint64_t getNextTime() { return currentTimestep++; }

int main() {
    std::cout << "[1] Parsing CSV into memory...\n";

    //load orders into ram
    std::vector<Order> orders = CSVParser::parse("data/orders.csv");

    if (orders.empty()) {
        std::cerr << "ERROR: No orders loaded. Check if the data/orders.csv exists\n";
        return 1;
    }

    std::cout << "Loaded " << orders.size() << " orders\n";
    std::cout << "[2] Starting Matching Engine...\n";

    OrderBook lob;

    //Start clock
    auto start = std::chrono::high_resolution_clock::now();

    //sim loop
    for (const auto& order : orders) {
        lob.addOrder(order);
    }

    auto end = std::chrono::high_resolution_clock::now();

    //calculate elapsed time in ms
    std::chrono::duration<double, std::micro> elapsed = end - start;

    //throughput -> orders per sec
    double seconds = elapsed.count() / 1'000'000.0;
    double ops = orders.size() / seconds;

    std::cout << "\n========== PERFORMANCE METRICS ==========\n";
    std::cout << "Time elapsed: " << elapsed.count() << " microseconds\n";
    std::cout << "Throughput:   " << ops << " orders / second\n";
    std::cout << "=========================================\n";    

    //lob.printBook()

    return 0;
}