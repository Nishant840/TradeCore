#include "../include/EngineRunner.h"
#include <variant>

EngineRunner::EngineRunner(TradeCallback callback)
    : engine(callback) {}

EngineRunner::~EngineRunner(){
    stop();
}

void EngineRunner::start(){
    running.store(true, std::memory_order_relaxed);
    workerThread = std::thread(&EngineRunner::runLoop, this);
}

void EngineRunner::stop(){
    if(!running.load(std::memory_order_relaxed)){
        return;
    }

    commandQueue.push(ShutdownCommand{});

    if(workerThread.joinable()){
        workerThread.join();
    }

    running.store(false, std::memory_order_relaxed);
}

void EngineRunner::submitOrder(Order order){
    commandQueue.push(NewOrderCommand{ std::move(order) });
}

void EngineRunner::submitCancel(uint64_t orderId){
    commandQueue.push(CancelOrderCommand{ orderId });
}

const MatchingEngine& EngineRunner::getEngine() const {
    return engine;
}

void EngineRunner::runLoop() {
    bool shuttingDown = false;

    while(!shuttingDown){
        Command cmd = commandQueue.waitAndPop();

        std::visit([this, &shuttingDown](auto&& c){
            using T = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<T, NewOrderCommand>) {
                Order order = c.order;
                engine.processOrder(order);
            }
            else if constexpr (std::is_same_v<T, CancelOrderCommand>) {
                engine.cancelOrder(c.orderId);
            }
            else if constexpr (std::is_same_v<T, ShutdownCommand>) {
                shuttingDown = true;
            }
        }, cmd);
    }
}