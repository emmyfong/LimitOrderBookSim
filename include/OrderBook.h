#pragma once

#include "Order.h"
#include <map>
#include <list>
#include <unordered_map>
#include <functional>
#include <cstdint>

//Outcome of an addOrder call.
enum class OrderStatus {
    Filled,          //fully filled immediately
    PartiallyFilled, //some quantity filled; see `reason` if the remainder was rejected instead of resting
    Accepted,        //nothing filled yet; the order rests in the book unfilled
    Rejected         //nothing filled, and nothing rests in the book
};

struct OrderResult {
    OrderStatus status;
    uint32_t filledQuantity = 0;
    const char* reason = nullptr; //non-null when status == Rejected, or a PartiallyFilled remainder didn't rest
};

//Outcome of a cancelOrder call.
enum class CancelStatus {
    Cancelled, //the order was resting and has been removed
    NotFound   //unknown id, already filled, or already cancelled - safe no-op
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

    void printBook() const;

private:
    //Outcome of one matching pass triggered by a newly inserted limit order.
    struct MatchOutcome {
        uint32_t filledQuantity = 0;
        bool selfTradeBlocked = false;
    };

    //internal matching alg for new orders
    MatchOutcome matchOrders(OrderSide incomingSide);

    //logic for market orders
    OrderResult executeMarketOrder(const Order& order);

    //Buyers are sorted in decending order (highest price first)
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
};
