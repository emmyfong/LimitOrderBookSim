#pragma once

#include <cstdint>
#include <string>

//order direction
enum class OrderSide {
    BUY,
    SELL
};

//order type
enum class OrderType {
    LIMIT,
    MARKET
};

//data structure for a single trade
struct Order {
    uint64_t orderId;
    uint64_t traderId;
    OrderSide side;
    OrderType type;
    uint64_t price;
    uint32_t quantity;
    uint64_t timestamp;
};