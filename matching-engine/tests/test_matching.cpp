#include "catch_amalgamated.hpp"
#include "../include/MatchingEngine.h"
#include <iostream>

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

TEST_CASE("Metrics track order count, trade count, and latency") {
    std::vector<Trade> trades;
    MatchingEngine engine([&](const Trade& t) { trades.push_back(t); });

    Order sell = makeOrder(301, Side::SELL, OrderType::LIMIT, 100.0, 10);
    Order buy  = makeOrder(302, Side::BUY,  OrderType::LIMIT, 100.0, 10);

    engine.processOrder(sell);
    engine.processOrder(buy);

    REQUIRE(engine.getMetrics().getTotalOrders() == 2);
    REQUIRE(engine.getMetrics().getTotalTrades() == 1);
    REQUIRE(engine.getMetrics().p50() >= 0);
    REQUIRE(engine.getMetrics().p99() >= engine.getMetrics().p50());
}

#include "../include/EngineRunner.h"
#include <thread>
#include <vector>
#include <atomic>

TEST_CASE("Concurrent stress test - quantity conservation under multiple producer threads"){
    std::atomic<uint64_t> totalTradedQty{0};
    std::atomic<uint64_t> tradeCount{0};

    EngineRunner runner([&](const Trade& t) {
        totalTradedQty.fetch_add(t.quantity, std::memory_order_relaxed);
        tradeCount.fetch_add(1, std::memory_order_relaxed);
    });

    auto startTime = std::chrono::high_resolution_clock::now();

    runner.start();

    const int numThreads      = 4;
    const int ordersPerThread = 1250;
    const double price        = 100.0;

    std::vector<std::thread> producers;

    for (int t = 0; t < numThreads; ++t) {
        producers.emplace_back([&runner, t, ordersPerThread, price]() {
            uint64_t baseId = static_cast<uint64_t>(t) * ordersPerThread * 2 + 1;

            for (int i = 0; i < ordersPerThread; ++i) {
                Order buy;
                buy.orderId   = baseId + i * 2;
                buy.userId    = "buyer";
                buy.side      = Side::BUY;
                buy.type      = OrderType::LIMIT;
                buy.price     = toTicks(price);
                buy.quantity  = 1;
                buy.timestamp = 0;

                Order sell;
                sell.orderId   = baseId + i * 2 + 1;
                sell.userId    = "seller";
                sell.side      = Side::SELL;
                sell.type      = OrderType::LIMIT;
                sell.price     = toTicks(price);
                sell.quantity  = 1;
                sell.timestamp = 0;

                runner.submitOrder(buy);
                runner.submitOrder(sell);
            }
        });
    }

    for (auto& th : producers) {
        th.join();
    }

    runner.stop();

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;

    uint64_t totalOrdersSubmitted = numThreads * ordersPerThread * 2;
    uint64_t expectedTradedQty    = numThreads * ordersPerThread;

    std::cout << "--- RAW C++ ENGINE BENCHMARK ---" << std::endl;
    std::cout << "Total Orders: " << totalOrdersSubmitted << std::endl;
    std::cout << "Elapsed Time: " << elapsed.count() << " seconds" << std::endl;
    std::cout << "Throughput  : " << (totalOrdersSubmitted / elapsed.count()) << " orders/sec" << std::endl;
    std::cout << "--------------------------------" << std::endl;

    REQUIRE(runner.getEngine().getMetrics().getTotalOrders() == totalOrdersSubmitted);
    REQUIRE(totalTradedQty.load() == expectedTradedQty);
    REQUIRE(tradeCount.load()     >= 1);
    REQUIRE(runner.getEngine().getOrderBook().hasBids() == false);
    REQUIRE(runner.getEngine().getOrderBook().hasAsks() == false);
}