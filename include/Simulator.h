#pragma once
#include "OrderBook.h"
#include "ThreadSafeQueue.h"
#include <string>
#include <thread>

class Simulator {
public:
    Simulator(const std:: string& filename);
    void run();

private:
    void producerLoop();
    void consumerLoop();

    std::string filename_;
    OrderBook lob_;
    ThreadSafeQueue<Order> orderQueue_;

};