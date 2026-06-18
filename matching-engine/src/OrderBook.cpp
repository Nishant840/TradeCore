#include "../include/OrderBook.h"

void OrderBook::addOrder(const Order& order) {
    orderLookup[order.orderId] = order;

    if(order.side == Side::BUY){
        bids[order.price].push(order);
    }
    else{
        asks[order.price].push(order);
    }
}

bool OrderBook::cancelOrder(uint64_t orderId){
    auto it = orderLookup.find(orderId);
    if(it == orderLookup.end()){
        return false;
    }

    Order& order = it->second;

    if(order.side == Side::BUY){
        auto levelIt = bids.find(order.price);
        if(levelIt != bids.end()){
            std::queue<Order> filtered;
            while(!levelIt->second.empty()){
                Order front = levelIt->second.front();
                levelIt->second.pop();
                if(front.orderId != orderId){
                    filtered.push(front);
                }
            }
            if(filtered.empty()){
                bids.erase(levelIt);
            }
            else{
                levelIt->second = filtered;
            }
        }
    }
    else{
        auto levelIt = asks.find(order.price);
        if(levelIt != asks.end()){
            std::queue<Order> filtered;
            while(!levelIt->second.empty()){
                Order front = levelIt->second.front();
                levelIt->second.pop();
                if(front.orderId != orderId){
                    filtered.push(front);
                }
            }
            if(filtered.empty()){
                asks.erase(levelIt);
            }
            else{
                levelIt->second = filtered;
            }
        }
    }

    orderLookup.erase(it);
    return true;
}

std::map<double, PriceLevel, std::greater<double>>& OrderBook::getBids(){
    return bids;
}

std::map<double, PriceLevel>& OrderBook::getAsks(){
    return asks;
}

bool OrderBook::hasBids() const {
    return !bids.empty();
}

bool OrderBook::hasAsks() const {
    return !asks.empty();
}

double OrderBook::bestBid() const {
    return bids.begin()->first;
}

double OrderBook::bestAsk() const {
    return asks.begin()->first;
}