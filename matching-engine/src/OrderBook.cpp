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
        bids.erase(order.price);
    }
    else{
        asks.erase(order.price);
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