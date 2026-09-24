#pragma once

#include "Order.h"
#include <map>
#include <list>
#include <unordered_map>
#include <vector>
#include <functional>
#include <cstdint>

//one executed trade
struct Trade {
    uint64_t buyOrderId;
    uint64_t sellOrderId;
    uint64_t buyTraderId;
    uint64_t sellTraderId;
    uint64_t price;
    uint32_t quantity;
};

//result of an addOrder call
enum class OrderStatus {
    Filled,
    PartiallyFilled,
    Accepted, //rests in the book, unfilled
    Rejected  //nothing filled, nothing rests
};

struct OrderResult {
    OrderStatus status;
    uint32_t filledQuantity = 0;
    const char* reason = nullptr; //set on Rejected, or a PartiallyFilled that didn't rest
};

//result of a cancelOrder call
enum class CancelStatus {
    Cancelled,
    NotFound //unknown, already filled, or already cancelled
};

class OrderBook {
public:
    OrderBook() = default;
    ~OrderBook() = default;

    //Core
    OrderResult addOrder(const Order& order);
    CancelStatus cancelOrder(uint64_t orderId);

    //Queries
    bool hasBids() const;
    bool hasAsks() const;
    uint64_t getBestBid() const;
    uint64_t getBestAsk() const;
    const std::vector<Trade>& getTrades() const;

    void printBook() const;

private:
    //outcome of one matching pass
    struct MatchOutcome {
        uint32_t filledQuantity = 0;
        bool selfTradeBlocked = false;
    };

    //internal matching alg for new orders
    MatchOutcome matchOrders(OrderSide incomingSide);

    //logic for market orders
    OrderResult executeMarketOrder(const Order& order);

    //Buyers are sorted in descending order (highest price first)
    //std::greater sorts highest key first
    std::map<uint64_t, std::list<Order>, std::greater<uint64_t>> bids_;

    //Sellers are sorted in ascending order (lowest price first)
    std::map<uint64_t, std::list<Order>, std::less<uint64_t>> asks_;

    //Where a resting order lives -> cancelOrder can find it without scanning whole book
    //Kept in sync everywhere an order is added or removed
    struct OrderLocation {
        OrderSide side;
        uint64_t price;
        std::list<Order>::iterator it;
    };
    std::unordered_map<uint64_t, OrderLocation> orderIndex_;

    //executed trades, in execution order
    std::vector<Trade> trades_;
};
