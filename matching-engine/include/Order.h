#pragma once

#include<cstdint>
#include<string>

// 1 tick = 0.01 units
// $100.10 = 10010 ticks
// Multiply human price by PRICE_SCALE before storing
static constexpr int64_t PRICE_SCALE = 100;

inline int64_t toTicks(double humanPrice){
    return static_cast<int64_t>(humanPrice*PRICE_SCALE+0.5);
}

inline double toHuman(int64_t ticks){
    return static_cast<double>(ticks) / PRICE_SCALE;
}

enum class Side {
    BUY,
    SELL
};

enum class OrderType {
    LIMIT,
    MARKET
};

struct Order {
    uint64_t    orderId;
    std::string userId;
    Side        side;
    OrderType   type;
    int64_t      price;
    uint64_t    quantity;
    uint64_t    timestamp;
};