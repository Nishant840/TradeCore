#pragma once

#include "Order.h"

#include<map>
#include<queue>
#include<unordered_map>
#include<functional>
#include<cstdint>

using PriceLevel = std::queue<Order>;

class OrderBook {
  public:
    void addOrder(const Order& order);
    bool cancelOrder(uint64_t orderId);

    std::map<double, PriceLevel, std::greater<double>>& getBids();
    std::map<double, PriceLevel>& getAsks();

    bool hasBids() const;
    bool hasAsks() const;

    double bestBid() const;
    double bestAsk() const;

  private:
    std::map<double, PriceLevel, std::greater<double>> bids;
    std::map<double, PriceLevel>                       asks;
    std::unordered_map<uint64_t, Order>                orderLookup;
};