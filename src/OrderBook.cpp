#include "OrderBook.h"
#include <iostream>
#include <algorithm> //for std::min
#include <iterator>  //for std::prev

void OrderBook::addOrder(const Order& order) {
    //Get the market order
    if (order.type == OrderType::MARKET) {
        executeMarketOrder(order);
        return;
    }

    //limit orders proceed
    if (order.side == OrderSide::BUY) {
        auto& queue = bids_[order.price];
        queue.push_back(order);
        orderIndex_[order.orderId] = {OrderSide::BUY, order.price, std::prev(queue.end())};
    } else {
        auto& queue = asks_[order.price];
        queue.push_back(order);
        orderIndex_[order.orderId] = {OrderSide::SELL, order.price, std::prev(queue.end())};
    }

    matchOrders();
}

void OrderBook::matchOrders() {
    //As long as there's a buyer and a seller -> try to match
    while (!bids_.empty() && !asks_.empty()) {
        //Get iterators to get the best price levels
        auto bestBidIter = bids_.begin();
        auto bestAskIter = asks_.begin();

        uint64_t bestBidPrice = bestBidIter->first;
        uint64_t bestAskPrice = bestAskIter->first;

        //Check for price crossover (spread > 0 == no trade)
        if (bestBidPrice < bestAskPrice) {
            //The highest buyer won't pay what the lowest is asking
            break;
        }

        //Get queue of orders at the price levels by reference
        auto& bidQueue = bestBidIter->second;
        auto& askQueue = bestAskIter->second;

        //Get the order at front of queue (time priority thing)
        Order& topBid = bidQueue.front();
        Order& topAsk = askQueue.front();

        //Execute trade for max possible overlap quantity
        uint32_t tradeQuantity = std::min(topBid.quantity, topAsk.quantity);

        topBid.quantity -= tradeQuantity;
        topAsk.quantity -= tradeQuantity;

        std::cout << "TRADE EXECUTED: " << tradeQuantity << " shares at $" << bestAskPrice << "\n";

        //clean memeory
        if (topBid.quantity == 0) {
            orderIndex_.erase(topBid.orderId);
            bidQueue.pop_front();
        }
        if (topAsk.quantity == 0) {
            orderIndex_.erase(topAsk.orderId);
            askQueue.pop_front();
        }

        if (bidQueue.empty()) {
            bids_.erase(bestBidIter);
        }
        if (askQueue.empty()) {
            asks_.erase(bestAskIter);
        }
    }
}

void OrderBook::cancelOrder(uint64_t orderId) {
    auto indexIt = orderIndex_.find(orderId);
    if (indexIt == orderIndex_.end()) {
        //unknown id, already filled, or already cancelled - safe no-op
        return;
    }

    const OrderLocation& location = indexIt->second;

    //orderIndex_ is kept in sync everywhere the book is mutated, so the
    //price level below is guaranteed to exist.
    if (location.side == OrderSide::BUY) {
        auto priceLevelIt = bids_.find(location.price);
        auto& queue = priceLevelIt->second;
        queue.erase(location.it);
        if (queue.empty()) {
            bids_.erase(priceLevelIt);
        }
    } else {
        auto priceLevelIt = asks_.find(location.price);
        auto& queue = priceLevelIt->second;
        queue.erase(location.it);
        if (queue.empty()) {
            asks_.erase(priceLevelIt);
        }
    }

    orderIndex_.erase(indexIt);
}

bool OrderBook::hasBids() const { return !bids_.empty(); }
bool OrderBook::hasAsks() const { return !asks_.empty(); }

uint64_t OrderBook::getBestBid() const {
    return hasBids() ? bids_.begin()->first : 0;
}

uint64_t OrderBook::getBestAsk() const {
    return hasAsks() ? asks_.begin()->first : 0;
}

void OrderBook::printBook() const {
    std::cout << "======== LIMIT ORDER BOOK SIM ========\n";
    std::cout << "--- ASKS ---\n";
    for (auto it = asks_.rbegin(); it != asks_.rend(); ++it) {
        uint32_t totalVol = 0;
        for (const auto& order : it->second) totalVol += order.quantity;
        std::cout << "$" << it->first << " | Vol: " << totalVol << " | Orders: " << it->second.size() << "\n";   
    }

    std::cout << "--- BIDS ---\n";
    for (const auto& pair : bids_) {
        uint32_t totalVolume = 0;
        for (const auto& order : pair.second) totalVolume += order.quantity;
        std::cout << "$" << pair.first << " | Vol: " << totalVolume << " | Orders: " << pair.second.size() << "\n";
    }
    std::cout << "======================================\n";
}

void OrderBook::executeMarketOrder(const Order& incomingOrder) {
    uint32_t remainingQuanitity = incomingOrder.quantity;

    if (incomingOrder.side == OrderSide::BUY) {
        //A market buy looks through the asks
        while (remainingQuanitity > 0 && !asks_.empty()) {
            auto bestAskIter = asks_.begin();
            auto& askQueue = bestAskIter->second;
            Order& topAsk = askQueue.front();

            uint32_t tradeQuantity = std::min(remainingQuanitity, topAsk.quantity);
            remainingQuanitity -= tradeQuantity;
            topAsk.quantity -= tradeQuantity;

            if (topAsk.quantity == 0) {
                orderIndex_.erase(topAsk.orderId);
                askQueue.pop_front();
            }
            if (askQueue.empty()) {
                asks_.erase(bestAskIter);
            }
        }
    } else {
        //market sell goes through the bids
        while (remainingQuanitity > 0 && !bids_.empty()) {
            auto bestBidIter = bids_.begin();
            auto& bidQueue = bestBidIter->second;
            Order& topBid = bidQueue.front();

            uint32_t tradeQuantity = std::min(remainingQuanitity, topBid.quantity);
            remainingQuanitity -= tradeQuantity;
            topBid.quantity -= tradeQuantity;

            if (topBid.quantity == 0) {
                orderIndex_.erase(topBid.orderId);
                bidQueue.pop_front();
            }
            if (bidQueue.empty()) {
                bids_.erase(bestBidIter);
            }
        }
    }
}