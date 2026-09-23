#pragma once

#include "Order.h"
#include <map>
#include <list>
#include <unordered_map>
#include <functional>
#include <cstdint>

class OrderBook {
public:
    OrderBook() = default;
    ~OrderBook() = default;

    //Core
    void addOrder(const Order& order);
    void cancelOrder(uint64_t orderId);

    //Queries
    bool hasBids() const;
    bool hasAsks() const;
    uint64_t getBestBid() const;
    uint64_t getBestAsk() const;

    void printBook() const;

private:
    //internal matching alg for new orders
    void matchOrders();

    //logic for market orders
    void executeMarketOrder(const Order& order);

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