#include "catch_amalgamated.hpp"
#include "../include/MatchingEngine.h"

uint64_t now(){
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

Order makeOrder(uint64_t id, Side side, OrderType type, double price, uint64_t qty){
    Order o;
    o.orderId   = id;
    o.userId    = "user_" + std::to_string(id);
    o.side      = side;
    o.type      = type;
    o.price     = price;
    o.quantity  = qty;
    o.timestamp = now();
    return o;
}

TEST_CASE("Full fill - limit buy matches limit sell"){
    std::vector<Trade> trades;
    MatchingEngine engine([&](const Trade& t){
        trades.push_back(t);
    });

    Order sell = makeOrder(1, Side::SELl, OrderType::LIMIT, 100.0, 10);
    Order buy  = makeOrder(2, Side::BUY, OrderType::LIMIT, 100.0, 10);

    engine.processOrder(sell);
    engine.processOrder(buy);

    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].quantity == 10);
    REQUIRE(trades[0].price == 100.0);
}

TEST_CASE("No match - buy price below ask"){
    std::vector<Trade> trades;
    MatchingEngine engine([&](const Trade& t){
        trades.push_back(t);
    });

    Order sell = makeOrder(1, Side::SELl, OrderType::LIMIT, 105.0, 10);
    Order buy  = makeOrder(2, Side::BUY, OrderType::LIMIT, 100.0, 10);

    engine.processOrder(sell);
    engine.processOrder(buy);

    REQUIRE(trades.size() == 0);
}

TEST_CASE("Partial fill - buy quantity greater than sell"){
    std::vector<Trade> trades;
    MatchingEngine engine([&](const Trade& t){
        trades.push_back(t);
    });

    Order sell = makeOrder(1, Side::SELl, OrderType::LIMIT, 100.0, 5);
    Order buy  = makeOrder(2, Side::BUY, OrderType::LIMIT, 100.0, 10);

    engine.processOrder(sell);
    engine.processOrder(buy);

    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].quantity == 5);
    REQUIRE(engine.getOrderBook().hasBids() == true);
}

TEST_CASE("Market buy fills against resting sells"){
    std::vector<Trade> trades;
    MatchingEngine engine([&](const Trade& t){
        trades.push_back(t);
    });

    Order sell = makeOrder(1, Side::SELl, OrderType::LIMIT, 100.0, 10);
    Order buy  = makeOrder(2, Side::BUY, OrderType::MARKET, 0.0, 10);

    engine.processOrder(sell);
    engine.processOrder(buy);

    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].quantity == 10);
}

TEST_CASE("Market buy on empty book - no trades"){
    std::vector<Trade> trades;
    MatchingEngine engine([&](const Trade& t){
        trades.push_back(t);
    });

    Order buy = makeOrder(1, Side::BUY, OrderType::MARKET, 0.0, 10);
    engine.processOrder(buy);

    REQUIRE(trades.size() == 0);
}

TEST_CASE("Multiple resting orders - FIFO priority"){
    std::vector<Trade> trades;
    MatchingEngine engine([&](const Trade& t){
        trades.push_back(t);
    });

    Order sell1 = makeOrder(1, Side::SELl, OrderType::LIMIT, 100.0, 5);
    Order sell2 = makeOrder(2, Side::SELl, OrderType::LIMIT, 100.0, 5);
    Order buy   = makeOrder(3, Side::BUY, OrderType::LIMIT, 100.0, 10);

    engine.processOrder(sell1);
    engine.processOrder(sell2);
    engine.processOrder(buy);
    
    REQUIRE(trades.size() == 2);
    REQUIRE(trades[0].sellOrderId == 1);
    REQUIRE(trades[1].sellOrderId == 2);
}

TEST_CASE("Cancel order - removed from book"){
    std::vector<Trade> trades;
    MatchingEngine engine([&](const Trade& t){
        trades.push_back(t);
    });

    Order sell = makeOrder(1, Side::SELl, OrderType::LIMIT, 100.0, 10);
    engine.processOrder(sell);
    engine.cancelOrder(1);

    Order buy = makeOrder(2, Side::BUY, OrderType::LIMIT, 100.0, 10);
    engine.processOrder(buy);

    REQUIRE(trades.size() == 0);
}