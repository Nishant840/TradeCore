#pragma once

#include "Order.h"

#include<map>
#include<queue>
#include<unordered_map>
#include<functional>
#include<cstdint>
#include<list>

struct OrderNode{
    Order order;
};

using PriceLevel    = std::list<OrderNode>;
using PriceLevelMap = std::map<int64_t, PriceLevel, std::greater<int64_t>>;
using AskLevelMap   = std::map<int64_t, PriceLevel>;
using OrderIterator = std::list<OrderNode>::iterator;

class OrderBook {
  public:
    void addOrder(const Order& order);
    bool cancelOrder(uint64_t orderId);
    void removeFilledOrder(uint64_t orderId, Side side, int64_t price);

    PriceLevelMap& getBids();
    AskLevelMap& getAsks();

    bool hasBids() const;
    bool hasAsks() const;

    int64_t bestBid() const;
    int64_t bestAsk() const;

  private:
    PriceLevelMap bids;
    AskLevelMap   asks;

    struct OrderMeta{
      OrderIterator iterator;
      Side          side;
      int64_t       price;
    };

    std::unordered_map<uint64_t, OrderMeta> orderLookup;
};