#include "catch_amalgamated.hpp"
#include "../include/MatchingEngine.h"

uint64_t nowMicros(){
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

Order makeOrder(uint64_t id, Side side, OrderType type, double humanPrice, uint64_t qty){
    Order o;
    o.orderId   = id;
    o.userId    = "user_" + std::to_string(id);
    o.side      = side;
    o.type      = type;
    o.price     = toTicks(humanPrice);
    o.quantity  = qty;
    o.timestamp = nowMicros();
    return o;
}

TEST_CASE("Full fill - limit buy matches limit sell"){
    std::vector<Trade> trades;
    MatchingEngine engine([&](const Trade& t){
        trades.push_back(t);
    });

    Order sell = makeOrder(1, Side::SELL, OrderType::LIMIT, 100.0, 10);
    Order buy  = makeOrder(2, Side::BUY, OrderType::LIMIT, 100.0, 10);

    engine.processOrder(sell);
    engine.processOrder(buy);

    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].quantity == 10);
    REQUIRE(trades[0].price == toTicks(100.0));
}

TEST_CASE("No match - buy price below ask"){
    std::vector<Trade> trades;
    MatchingEngine engine([&](const Trade& t){
        trades.push_back(t);
    });

    Order sell = makeOrder(1, Side::SELL, OrderType::LIMIT, 105.0, 10);
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

    Order sell = makeOrder(1, Side::SELL, OrderType::LIMIT, 100.0, 5);
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

    Order sell = makeOrder(1, Side::SELL, OrderType::LIMIT, 100.0, 10);
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

    Order sell1 = makeOrder(1, Side::SELL, OrderType::LIMIT, 100.0, 5);
    Order sell2 = makeOrder(2, Side::SELL, OrderType::LIMIT, 100.0, 5);
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

    Order sell = makeOrder(1, Side::SELL, OrderType::LIMIT, 100.0, 10);
    engine.processOrder(sell);
    engine.cancelOrder(1);

    Order buy = makeOrder(2, Side::BUY, OrderType::LIMIT, 100.0, 10);
    engine.processOrder(buy);

    REQUIRE(trades.size() == 0);
}

TEST_CASE("Cancel on fully filled order does not crash or double-trade") {
    std::vector<Trade> trades;
    MatchingEngine engine([&](const Trade& t){ 
        trades.push_back(t);
    });

    Order sell = makeOrder(101, Side::SELL, OrderType::LIMIT, 60000.0, 1);
    engine.processOrder(sell);

    Order buy = makeOrder(102, Side::BUY, OrderType::MARKET, 0.0, 1);
    engine.processOrder(buy);

    REQUIRE(trades.size() == 1);

    bool result = engine.cancelOrder(101);

    REQUIRE(result == false);
}

TEST_CASE("Duplicate orderId on add is rejected, not silently overwritten") {
    std::vector<Trade> trades;
    MatchingEngine engine([&](const Trade& t) { trades.push_back(t); });

    Order first  = makeOrder(201, Side::SELL, OrderType::LIMIT, 60000.0, 1);
    bool firstOk = engine.processOrder(first);

    Order duplicate  = makeOrder(201, Side::SELL, OrderType::LIMIT, 61000.0, 1);
    bool duplicateOk = engine.processOrder(duplicate);

    REQUIRE(firstOk     == true);
    REQUIRE(duplicateOk == false);

    bool cancelled = engine.cancelOrder(201);
    REQUIRE(cancelled == true);

    bool cancelledAgain = engine.cancelOrder(201);
    REQUIRE(cancelledAgain == false);
}