#pragma once

#include "Order.h"
#include <cstdint>
#include <variant>

struct NewOrderCommand {
    Order order;
};

struct CancelOrderCommand {
    uint64_t orderId;
};

struct ShutdownCommand {};

using Command = std::variant<NewOrderCommand, CancelOrderCommand, ShutdownCommand>;