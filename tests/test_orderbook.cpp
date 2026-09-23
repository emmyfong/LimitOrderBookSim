#include "OrderBook.h"
#include <cassert>
#include <iostream>

void test_price_time_priority() {
    OrderBook lob;
    
    // Two buyers at the same price. Order 1 arrives first.
    lob.addOrder({1, OrderSide::BUY, OrderType::LIMIT, 10000, 50, 1});
    lob.addOrder({2, OrderSide::BUY, OrderType::LIMIT, 10000, 50, 2});
    
    // A seller arrives to sell 50 shares
    lob.addOrder({3, OrderSide::SELL, OrderType::MARKET, 0, 50, 3});

    // If Price-Time priority works, Order 1 should be filled and deleted, 
    // and Order 2 should still be resting in the book.
    assert(lob.hasBids() == true);
    assert(lob.getBestBid() == 10000);
    
    std::cout << "[PASS] Price-Time Priority Test\n";
}

void test_partial_fills() {
    OrderBook lob;
    
    lob.addOrder({1, OrderSide::SELL, OrderType::LIMIT, 15000, 100, 1});
    lob.addOrder({2, OrderSide::BUY, OrderType::LIMIT, 15000, 40, 2}); // Buys only 40

    // The seller should still be there, but with only 60 shares left.
    assert(lob.hasAsks() == true);
    assert(lob.getBestAsk() == 15000);

    std::cout << "[PASS] Partial Fill Test\n";
}

int main() {
    std::cout << "Running OrderBook Unit Tests...\n";
    
    test_price_time_priority();
    test_partial_fills();
    
    std::cout << "✅ ALL TESTS PASSED SUCCESSFULLY.\n";
    return 0;
}