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

void test_market_sell_executes_against_bids() {
    OrderBook lob;

    // A resting bid at $150.00
    lob.addOrder({1, OrderSide::BUY, OrderType::LIMIT, 15000, 50, 1});

    // A market sell should immediately match it and fully fill the bid
    lob.addOrder({2, OrderSide::SELL, OrderType::MARKET, 0, 50, 2});

    assert(lob.hasBids() == false);

    std::cout << "[PASS] Market Sell Executes Against Bids Test\n";
}

void test_cancel_ask_order() {
    OrderBook lob;

    lob.addOrder({1, OrderSide::SELL, OrderType::LIMIT, 15000, 100, 1});
    lob.cancelOrder(1);

    // Canceling the only resting ask should remove it from the book
    assert(lob.hasAsks() == false);

    std::cout << "[PASS] Cancel Ask Order Test\n";
}

void test_cancel_bid_removes_empty_price_level() {
    OrderBook lob;

    lob.addOrder({1, OrderSide::BUY, OrderType::LIMIT, 15000, 100, 1});
    lob.cancelOrder(1);

    // Canceling the only resting bid should remove the now-empty price level
    assert(lob.hasBids() == false);

    std::cout << "[PASS] Cancel Bid Removes Empty Price Level Test\n";
}

void test_cancel_unknown_order_is_safe() {
    OrderBook lob;
    lob.addOrder({1, OrderSide::BUY, OrderType::LIMIT, 15000, 50, 1});

    // Cancelling an id that was never added, then cancelling the same id twice,
    // should both be safe no-ops rather than crashing or corrupting the book.
    lob.cancelOrder(999);
    lob.cancelOrder(1);
    lob.cancelOrder(1);

    assert(lob.hasBids() == false);

    std::cout << "[PASS] Cancel Unknown/Already-Cancelled Order Is Safe Test\n";
}

void test_cancel_preserves_other_orders_at_same_price_level() {
    OrderBook lob;

    lob.addOrder({1, OrderSide::BUY, OrderType::LIMIT, 15000, 50, 1});
    lob.addOrder({2, OrderSide::BUY, OrderType::LIMIT, 15000, 30, 2});

    lob.cancelOrder(1);

    // Order 2 should still be resting at the same price level
    assert(lob.hasBids() == true);
    assert(lob.getBestBid() == 15000);

    // A market sell for exactly order 2's remaining size should fully drain the level
    lob.addOrder({3, OrderSide::SELL, OrderType::MARKET, 0, 30, 3});
    assert(lob.hasBids() == false);

    std::cout << "[PASS] Cancel Preserves Other Orders At Same Price Level Test\n";
}

int main() {
    std::cout << "Running OrderBook Unit Tests...\n";

    test_price_time_priority();
    test_partial_fills();
    test_market_sell_executes_against_bids();
    test_cancel_ask_order();
    test_cancel_bid_removes_empty_price_level();
    test_cancel_unknown_order_is_safe();
    test_cancel_preserves_other_orders_at_same_price_level();

    std::cout << "ALL TESTS PASSED SUCCESSFULLY.\n";
    return 0;
}