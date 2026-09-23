#include "Simulator.h"
#include "CSVParser.h"
#include <iostream>
#include <chrono>
#include <vector>

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
        lob_.addOrder(order);
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
    std::chrono::duration<double, std::micro> elapsed = end - start;

    std::cout << "[*] Simulation Complete\n";
    std::cout << "Time Elapsed: " << elapsed.count() << " ms\n";
}