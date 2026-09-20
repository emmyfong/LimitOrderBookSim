#include "OrderBook.h"
#include <iostream>

//Helper vars
uint64_t currentTimestep = 1;
uint64_t getNextTime() { return currentTimestep++; }

int main() {
    std::cout << "\nInitializing Market...\n";

    OrderBook lob;
    //add sellers (asks) with prices in cents
    lob.addOrder({1, OrderSide::SELL, OrderType::LIMIT, 15000, 100, getNextTime()});
    lob.addOrder({2, OrderSide::SELL, OrderType::LIMIT, 15100, 50, getNextTime()});

    //buyers (bids)
    lob.addOrder({3, OrderSide::BUY, OrderType::LIMIT, 14900, 50, getNextTime()});
    lob.addOrder({4, OrderSide::BUY, OrderType::LIMIT, 14000, 75, getNextTime()});

    //Spread is currently 150 (ask) - 149 (bids) so no trade should happen
    lob.printBook();

    lob.cancelOrder(4);

    return 0;
}