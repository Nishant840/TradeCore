#include "../include/EngineRunner.h"
#include "../include/NetworkServer.h"

#include <iostream>
#include <csignal>
#include <atomic>

std::atomic<bool> keepRunning{true};

void signalHandler(int){
    keepRunning.store(false, std::memory_order_relaxed);
}

int main(){
    NetworkServer* serverPtr = nullptr;

    EngineRunner runner([&serverPtr](const Trade& trade){
        if(serverPtr != nullptr){
            serverPtr->broadcastTrade(trade);
        }
    });

    NetworkServer server(runner, 9000);
    serverPtr = &server;

    runner.start();
    server.start();

    std::signal(SIGINT, signalHandler);

    std::cout << "TradeCore engine running. Press Ctrl+C to stop." << std::endl;

    while(keepRunning.load(std::memory_order_relaxed)){
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout<<"Shutting down.."<<std::endl;

    server.stop();
    runner.stop();

    return 0;
}