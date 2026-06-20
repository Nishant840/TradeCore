#pragma once

#include "EngineRunner.h"
#include <thread>
#include <atomic>
#include <mutex>

class NetworkServer {
  public:
    NetworkServer(EngineRunner& runner, int port);
    ~NetworkServer();

    void start();
    void stop();

    void broadcastTrade(const Trade& trade);

  private:
    EngineRunner&       runner;
    int                 port;
    int                 listenFd;
    int                 clientFd;

    std::thread         acceptThread;
    std::atomic<bool>   running{false};
    std::mutex          clientWriteMutex;

    void acceptLoop();
    void handleClient(int fd);
};