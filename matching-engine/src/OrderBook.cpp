#include "../include/OrderBook.h"

void OrderBook::addOrder(const Order& order) {
    OrderNode node{order};

    if(order.side == Side::BUY){
        auto& level = bids[order.price];
        level.push_back(node);
        auto it = std::prev(level.end());
        orderLookup[order.orderId] = OrderMeta{it, Side::BUY, order.price};
    }
    else{
        auto& level = asks[order.price];
        level.push_back(node);
        auto it = std::prev(level.end());
        orderLookup[order.orderId] = OrderMeta{it, Side::SELL, order.price};
    }
}

bool OrderBook::cancelOrder(uint64_t orderId){
    auto metaIt = orderLookup.find(orderId);
    if(metaIt == orderLookup.end()){
        return false;
    }

    OrderMeta& meta = metaIt->second;

    if(meta.side == Side::BUY){
        auto& level = bids[meta.price];
        level.erase(meta.iterator);
        if(level.empty()){
            bids.erase(meta.price);
        }
    }
    else{
        auto& level = asks[meta.price];
        level.erase(meta.iterator);
        if(level.empty()){
            asks.erase(meta.price);
        }
    }

    orderLookup.erase(metaIt);
    return true;
}

void OrderBook::removeFilledOrder(uint64_t orderId, Side side, int64_t price){
    auto metaIt = orderLookup.find(orderId);
    if(metaIt == orderLookup.end()){
        return;
    }

    if(side == Side::BUY){
        auto& level = bids[price];
        level.erase(metaIt->second.iterator);
        if(level.empty()){
            bids.erase(price);
        }
    }
    else{
        auto& level = asks[price];
        level.erase(metaIt->second.iterator);
        if(level.empty()){
            asks.erase(price);
        }
    }

    orderLookup.erase(metaIt);
}

PriceLevelMap& OrderBook::getBids() {
    return bids;
}
AskLevelMap& OrderBook::getAsks(){
    return asks;
}

bool OrderBook::hasBids() const {
    return !bids.empty();
}

bool OrderBook::hasAsks() const {
    return !asks.empty();
}

int64_t OrderBook::bestBid() const {
    return bids.begin()->first;
}

int64_t OrderBook::bestAsk() const {
    return asks.begin()->first;
}