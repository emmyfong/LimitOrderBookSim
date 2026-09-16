#pragma once

#include "Order.h"
#include <map>
#include <list>
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

    //Buyers are sorted in decending order (highest price first)
    //std::greater sorts highest key first
    std::map<uint64_t, std::list<Order>, std::greater<uint64_t>> bids_;

    //Sellers are sorted in ascending order (lowest price first)
    std::map<uint64_t, std::list<Order>, std::less<uint64_t>> asks_;
    
};