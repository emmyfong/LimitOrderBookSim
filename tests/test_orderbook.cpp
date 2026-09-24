#include "OrderBook.h"
#include <cassert>
#include <iostream>

void test_price_time_priority() {
    OrderBook lob;

    // Two buyers at the same price. Order 1 arrives first.
    lob.addOrder({1, 100, OrderSide::BUY, OrderType::LIMIT, 10000, 50, 1});
    lob.addOrder({2, 200, OrderSide::BUY, OrderType::LIMIT, 10000, 50, 2});

    // A seller arrives to sell 50 shares
    lob.addOrder({3, 300, OrderSide::SELL, OrderType::MARKET, 0, 50, 3});

    // If Price-Time priority works, Order 1 should be filled and deleted,
    // and Order 2 should still be resting in the book.
    assert(lob.hasBids() == true);
    assert(lob.getBestBid() == 10000);

    std::cout << "[PASS] Price-Time Priority Test\n";
}

void test_partial_fills() {
    OrderBook lob;

    lob.addOrder({1, 100, OrderSide::SELL, OrderType::LIMIT, 15000, 100, 1});
    OrderResult result = lob.addOrder({2, 200, OrderSide::BUY, OrderType::LIMIT, 15000, 40, 2}); // Buys only 40

    // The seller should still be there, but with only 60 shares left.
    assert(lob.hasAsks() == true);
    assert(lob.getBestAsk() == 15000);

    // The buyer's whole 40 filled immediately against the resting seller.
    assert(result.status == OrderStatus::Filled);
    assert(result.filledQuantity == 40);

    std::cout << "[PASS] Partial Fill Test\n";
}

void test_market_sell_executes_against_bids() {
    OrderBook lob;

    // A resting bid at $150.00
    lob.addOrder({1, 100, OrderSide::BUY, OrderType::LIMIT, 15000, 50, 1});

    // A market sell should immediately match it and fully fill the bid
    OrderResult result = lob.addOrder({2, 200, OrderSide::SELL, OrderType::MARKET, 0, 50, 2});

    assert(lob.hasBids() == false);
    assert(result.status == OrderStatus::Filled);
    assert(result.filledQuantity == 50);

    std::cout << "[PASS] Market Sell Executes Against Bids Test\n";
}

void test_cancel_ask_order() {
    OrderBook lob;

    lob.addOrder({1, 100, OrderSide::SELL, OrderType::LIMIT, 15000, 100, 1});
    CancelStatus status = lob.cancelOrder(1);

    // Canceling the only resting ask should remove it from the book
    assert(status == CancelStatus::Cancelled);
    assert(lob.hasAsks() == false);

    std::cout << "[PASS] Cancel Ask Order Test\n";
}

void test_cancel_bid_removes_empty_price_level() {
    OrderBook lob;

    lob.addOrder({1, 100, OrderSide::BUY, OrderType::LIMIT, 15000, 100, 1});
    lob.cancelOrder(1);

    // Canceling the only resting bid should remove the now-empty price level
    assert(lob.hasBids() == false);

    std::cout << "[PASS] Cancel Bid Removes Empty Price Level Test\n";
}

void test_cancel_unknown_order_is_safe() {
    OrderBook lob;
    lob.addOrder({1, 100, OrderSide::BUY, OrderType::LIMIT, 15000, 50, 1});

    // Cancelling an id that was never added should report NotFound, not crash
    assert(lob.cancelOrder(999) == CancelStatus::NotFound);

    assert(lob.cancelOrder(1) == CancelStatus::Cancelled);
    // Cancelling the same id a second time should also be a safe no-op
    assert(lob.cancelOrder(1) == CancelStatus::NotFound);

    assert(lob.hasBids() == false);

    std::cout << "[PASS] Cancel Unknown/Already-Cancelled Order Is Safe Test\n";
}

void test_cancel_preserves_other_orders_at_same_price_level() {
    OrderBook lob;

    lob.addOrder({1, 100, OrderSide::BUY, OrderType::LIMIT, 15000, 50, 1});
    lob.addOrder({2, 200, OrderSide::BUY, OrderType::LIMIT, 15000, 30, 2});

    lob.cancelOrder(1);

    // Order 2 should still be resting at the same price level
    assert(lob.hasBids() == true);
    assert(lob.getBestBid() == 15000);

    // A market sell for exactly order 2's remaining size should fully drain the level
    lob.addOrder({3, 300, OrderSide::SELL, OrderType::MARKET, 0, 30, 3});
    assert(lob.hasBids() == false);

    std::cout << "[PASS] Cancel Preserves Other Orders At Same Price Level Test\n";
}

void test_self_trade_is_rejected_not_matched() {
    OrderBook lob;

    // Trader 100 rests a bid, then sends a crossing sell as the same trader.
    lob.addOrder({1, 100, OrderSide::BUY, OrderType::LIMIT, 15000, 50, 1});
    OrderResult result = lob.addOrder({2, 100, OrderSide::SELL, OrderType::LIMIT, 15000, 50, 2});

    // The incoming order must be rejected outright - no trade, nothing rests for it.
    assert(result.status == OrderStatus::Rejected);
    assert(result.filledQuantity == 0);
    assert(result.reason != nullptr);

    // The original resting bid is untouched (Cancel-Newest: reject the incoming order).
    assert(lob.hasBids() == true);
    assert(lob.getBestBid() == 15000);
    assert(lob.hasAsks() == false);

    std::cout << "[PASS] Self-Trade Is Rejected, Not Matched Test\n";
}

void test_self_trade_market_order_is_rejected() {
    OrderBook lob;

    lob.addOrder({1, 100, OrderSide::SELL, OrderType::LIMIT, 15000, 50, 1});
    OrderResult result = lob.addOrder({2, 100, OrderSide::BUY, OrderType::MARKET, 0, 50, 2});

    assert(result.status == OrderStatus::Rejected);
    assert(result.filledQuantity == 0);
    assert(lob.hasAsks() == true);

    std::cout << "[PASS] Self-Trade Market Order Is Rejected Test\n";
}

void test_different_traders_still_match_normally() {
    OrderBook lob;

    lob.addOrder({1, 100, OrderSide::SELL, OrderType::LIMIT, 15000, 50, 1});
    OrderResult result = lob.addOrder({2, 200, OrderSide::BUY, OrderType::LIMIT, 15000, 50, 2});

    // Different traders (100 vs 200) - this should fill normally.
    assert(result.status == OrderStatus::Filled);
    assert(result.filledQuantity == 50);
    assert(lob.hasAsks() == false);

    std::cout << "[PASS] Different Traders Still Match Normally Test\n";
}

void test_records_limit_match_as_a_trade() {
    OrderBook lob;

    lob.addOrder({1, 100, OrderSide::SELL, OrderType::LIMIT, 15000, 100, 1});
    lob.addOrder({2, 200, OrderSide::BUY, OrderType::LIMIT, 15000, 40, 2});

    const std::vector<Trade>& trades = lob.getTrades();
    assert(trades.size() == 1);
    assert(trades[0].buyOrderId == 2);
    assert(trades[0].sellOrderId == 1);
    assert(trades[0].buyTraderId == 200);
    assert(trades[0].sellTraderId == 100);
    assert(trades[0].price == 15000);
    assert(trades[0].quantity == 40);

    std::cout << "[PASS] Records Limit Match As A Trade Test\n";
}

void test_records_market_order_match_as_a_trade() {
    OrderBook lob;

    lob.addOrder({1, 100, OrderSide::BUY, OrderType::LIMIT, 15000, 50, 1});
    lob.addOrder({2, 200, OrderSide::SELL, OrderType::MARKET, 0, 50, 2});

    const std::vector<Trade>& trades = lob.getTrades();
    assert(trades.size() == 1);
    assert(trades[0].buyOrderId == 1);
    assert(trades[0].sellOrderId == 2);
    assert(trades[0].price == 15000);
    assert(trades[0].quantity == 50);

    std::cout << "[PASS] Records Market Order Match As A Trade Test\n";
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
    test_self_trade_is_rejected_not_matched();
    test_self_trade_market_order_is_rejected();
    test_different_traders_still_match_normally();
    test_records_limit_match_as_a_trade();
    test_records_market_order_match_as_a_trade();

    std::cout << "ALL TESTS PASSED SUCCESSFULLY.\n";
    return 0;
}
