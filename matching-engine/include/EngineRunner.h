#pragma once

#include "MatchingEngine.h"
#include "ThreadSafeQueue.h"
#include "Command.h"

#include <thread>
#include <atomic>

class EngineRunner {
  public:
    explicit EngineRunner(TradeCallback callback);
    ~EngineRunner();

    void start();
    void stop();

    void submitOrder(Order order);
    void submitCancel(uint64_t orderId);

    const MatchingEngine& getEngine() const;

  private:
    MatchingEngine                          engine;
    ThreadSafeQueue<Command>                commandQueue;
    std::thread                             workerThread;
    std::atomic<bool>                       running{false};

    void runLoop();
};