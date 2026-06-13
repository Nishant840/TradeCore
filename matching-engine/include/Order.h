#pragma once

#include<cstdint>
#include<string>

enum class Side {
    BUY,
    SELl
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
    double      price;
    uint64_t    quantity;
    uint64_t    timestamp;
};