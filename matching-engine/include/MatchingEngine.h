#pragma once

#include "Order.h"
#include "OrderBook.h"

#include<vector>
#include<functional>
#include<cstdint>

struct Trade{
    uint64_t tradeId;
    uint64_t buyOrderId;
    uint64_t sellOrderId;
    int64_t price;
    uint64_t quantity;
    uint64_t timestamp;
};

using TradeCallback = std::function<void(const Trade&)>;

class MatchingEngine{
  public:
    explicit MatchingEngine(TradeCallback callback);

    bool processOrder(Order& order);
    bool cancelOrder(uint64_t orderId);

    const OrderBook& getOrderBook() const;

  private:
    OrderBook       book;
    TradeCallback   onTrade;
    uint64_t        nextTradeId;

    void matchLimit(Order& order);
    void matchMarket(Order& order);

    Trade createTrade(const Order& buy,
                      const Order& sell,
                      int64_t price,
                      uint64_t quantity
                    );
};