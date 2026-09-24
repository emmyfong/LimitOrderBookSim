#include "OrderBook.h"
#include <iostream>
#include <algorithm> //for std::min
#include <iterator>  //for std::prev

namespace {
constexpr const char* kSelfTradeReason =
    "self-trade prevention: order would cross trader's own resting order";
constexpr const char* kNoLiquidityReason = "no resting liquidity available to match against";
} // namespace

OrderResult OrderBook::addOrder(const Order& order) {
    //Get the market order
    if (order.type == OrderType::MARKET) {
        return executeMarketOrder(order);
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

    MatchOutcome outcome = matchOrders(order.side);

    if (outcome.filledQuantity >= order.quantity) {
        return {OrderStatus::Filled, outcome.filledQuantity, nullptr};
    }
    if (outcome.selfTradeBlocked) {
        return outcome.filledQuantity == 0
            ? OrderResult{OrderStatus::Rejected, 0, kSelfTradeReason}
            : OrderResult{OrderStatus::PartiallyFilled, outcome.filledQuantity, kSelfTradeReason};
    }
    if (outcome.filledQuantity > 0) {
        return {OrderStatus::PartiallyFilled, outcome.filledQuantity, nullptr};
    }
    return {OrderStatus::Accepted, 0, nullptr};
}

OrderBook::MatchOutcome OrderBook::matchOrders(OrderSide incomingSide) {
    MatchOutcome outcome;

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

        //self-trade prevention (Cancel-Newest): stop the incoming order here
        //rather than trade against the trader's own resting order
        if (topBid.traderId == topAsk.traderId) {
            outcome.selfTradeBlocked = true;
            uint64_t incomingOrderId = (incomingSide == OrderSide::BUY) ? topBid.orderId : topAsk.orderId;
            cancelOrder(incomingOrderId);
            break;
        }

        //Execute trade for max possible overlap quantity
        uint32_t tradeQuantity = std::min(topBid.quantity, topAsk.quantity);
        outcome.filledQuantity += tradeQuantity;

        topBid.quantity -= tradeQuantity;
        topAsk.quantity -= tradeQuantity;

        //record the trade
        trades_.push_back({topBid.orderId, topAsk.orderId, topBid.traderId, topAsk.traderId,
                            bestAskPrice, tradeQuantity});

        //remove filled orders
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

    return outcome;
}

CancelStatus OrderBook::cancelOrder(uint64_t orderId) {
    auto indexIt = orderIndex_.find(orderId);
    if (indexIt == orderIndex_.end()) {
        //unknown id, already filled, or already cancelled - safe no-op
        return CancelStatus::NotFound;
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
    return CancelStatus::Cancelled;
}

bool OrderBook::hasBids() const { return !bids_.empty(); }
bool OrderBook::hasAsks() const { return !asks_.empty(); }

uint64_t OrderBook::getBestBid() const {
    return hasBids() ? bids_.begin()->first : 0;
}

uint64_t OrderBook::getBestAsk() const {
    return hasAsks() ? asks_.begin()->first : 0;
}

const std::vector<Trade>& OrderBook::getTrades() const {
    return trades_;
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

OrderResult OrderBook::executeMarketOrder(const Order& incomingOrder) {
    uint32_t remainingQuantity = incomingOrder.quantity;
    uint32_t filledQuantity = 0;
    bool selfTradeBlocked = false;

    if (incomingOrder.side == OrderSide::BUY) {
        //A market buy looks through the asks
        while (remainingQuantity > 0 && !asks_.empty()) {
            auto bestAskIter = asks_.begin();
            auto& askQueue = bestAskIter->second;
            Order& topAsk = askQueue.front();

            if (topAsk.traderId == incomingOrder.traderId) {
                selfTradeBlocked = true;
                break;
            }

            uint32_t tradeQuantity = std::min(remainingQuantity, topAsk.quantity);
            remainingQuantity -= tradeQuantity;
            filledQuantity += tradeQuantity;
            topAsk.quantity -= tradeQuantity;

            //executes at the resting order's price
            trades_.push_back({incomingOrder.orderId, topAsk.orderId, incomingOrder.traderId,
                                topAsk.traderId, topAsk.price, tradeQuantity});

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
        while (remainingQuantity > 0 && !bids_.empty()) {
            auto bestBidIter = bids_.begin();
            auto& bidQueue = bestBidIter->second;
            Order& topBid = bidQueue.front();

            if (topBid.traderId == incomingOrder.traderId) {
                selfTradeBlocked = true;
                break;
            }

            uint32_t tradeQuantity = std::min(remainingQuantity, topBid.quantity);
            remainingQuantity -= tradeQuantity;
            filledQuantity += tradeQuantity;
            topBid.quantity -= tradeQuantity;

            //executes at the resting order's price
            trades_.push_back({topBid.orderId, incomingOrder.orderId, topBid.traderId,
                                incomingOrder.traderId, topBid.price, tradeQuantity});

            if (topBid.quantity == 0) {
                orderIndex_.erase(topBid.orderId);
                bidQueue.pop_front();
            }
            if (bidQueue.empty()) {
                bids_.erase(bestBidIter);
            }
        }
    }

    if (filledQuantity >= incomingOrder.quantity) {
        return {OrderStatus::Filled, filledQuantity, nullptr};
    }
    if (selfTradeBlocked) {
        return filledQuantity == 0
            ? OrderResult{OrderStatus::Rejected, 0, kSelfTradeReason}
            : OrderResult{OrderStatus::PartiallyFilled, filledQuantity, kSelfTradeReason};
    }
    if (filledQuantity > 0) {
        return {OrderStatus::PartiallyFilled, filledQuantity, nullptr};
    }
    return {OrderStatus::Rejected, 0, kNoLiquidityReason};
}
