#include "../include/NetworkServer.h"
#include "../third_party/json.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include "../include/Metrics.h"

using json = nlohmann::json;

NetworkServer::NetworkServer(EngineRunner& runner, int port)
    : runner(runner), port(port), listenFd(-1), clientFd(-1) {}

NetworkServer::~NetworkServer(){
    stop();
}

void NetworkServer::start(){
    listenFd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = INADDR_ANY;
    addr.sin_port           = htons(port);

    bind(listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    listen(listenFd, 1);

    running.store(true, std::memory_order_relaxed);
    acceptThread = std::thread(&NetworkServer::acceptLoop, this);

    std::cout << "NetworkServer listening on port " << port << std::endl;
}

void NetworkServer::stop(){
    if(!running.load(std::memory_order_relaxed)){
        return;
    }
    running.store(false, std::memory_order_relaxed);

    if(listenFd != -1) close(listenFd);
    if(clientFd != -1) close(clientFd);
    
    if(acceptThread.joinable()){
        acceptThread.join();
    }
}

void NetworkServer::acceptLoop(){
    while(running.load(std::memory_order_relaxed)){
        int fd = accept(listenFd, nullptr, nullptr);
        if(fd < 0) continue;

        clientFd = fd;
        handleClient(fd);
    }
}

void NetworkServer::handleClient(int fd){
    std::string buffer;
    char chunk[4096];
    bool isFirstRead = true;

    while(running.load(std::memory_order_relaxed)){
        ssize_t bytesRead = read(fd, chunk, sizeof(chunk));
        if(bytesRead <= 0) break;

        if(isFirstRead) {
            isFirstRead = false;
            // If the first byte isn't a JSON bracket, it's an HTTP probe from Render. Drop it instantly!
            if(chunk[0] != '{'){
                std::cerr << "Dropped HTTP probe connection." << std::endl;
                break; 
            }
        }

        buffer.append(chunk, bytesRead);

        size_t newlinePos;
        while((newlinePos = buffer.find('\n')) != std::string::npos){
            std::string line = buffer.substr(0, newlinePos);
            buffer.erase(0, newlinePos+1);

            if(line.empty()) continue;

            try{
                json j = json::parse(line);
                std::string cmd = j.at("cmd").get<std::string>();

                if(cmd == "new_order"){
                    Order order;
                    order.orderId   = j.at("orderId").get<uint64_t>();
                    order.userId    = j.at("userId").get<std::string>();
                    order.side      = (j.at("side").get<std::string>() == "BUY") ? Side::BUY : Side::SELL;
                    order.type      = (j.at("type").get<std::string>() == "LIMIT") ? OrderType::LIMIT : OrderType::MARKET;
                    order.price     = toTicks(j.at("price").get<double>());
                    order.quantity  = j.at("quantity").get<uint64_t>();
                    order.timestamp = 0;

                    runner.submitOrder(order);
                }
                else if(cmd == "cancel"){
                    uint64_t orderId = j.at("orderId").get<uint64_t>();
                    runner.submitCancel(orderId);
                }
                else if(cmd == "get_book"){
                    std::string requestId = j.at("requestId").get<std::string>();
                    int depth = j.value("depth", 10);

                    auto bids = runner.getEngine().getOrderBook().getTopBids(depth);
                    auto asks = runner.getEngine().getOrderBook().getTopAsks(depth);

                    json response;
                    response["event"]       = "book_snapshot";
                    response["requestId"]   = requestId;

                    json bidsJson = json::array();
                    for(const auto& level: bids){
                        bidsJson.push_back({
                            {"price", toHuman(level.price)},
                            {"quantity", level.totalQuantity}
                        });
                    }

                    json asksJson = json::array();
                    for(const auto& level: asks){
                        asksJson.push_back({
                            {"price", toHuman(level.price)},
                            {"quantity", level.totalQuantity}
                        });
                    }

                    response["bids"] = bidsJson;
                    response["asks"] = asksJson;

                    std::string line = response.dump() + "\n";
                    std::lock_guard<std::mutex> lock(clientWriteMutex);
                    write(fd, line.c_str(), line.size());
                }
                else if (cmd == "get_metrics") {
                    std::string requestId = j.at("requestId").get<std::string>();

                    const Metrics& metrics = runner.getEngine().getMetrics();

                    json response;
                    response["event"]       = "metrics_snapshot";
                    response["requestId"]   = requestId;
                    response["totalOrders"] = metrics.getTotalOrders();
                    response["totalTrades"] = metrics.getTotalTrades();
                    response["p50Micros"]   = metrics.p50();
                    response["p95Micros"]   = metrics.p95();
                    response["p99Micros"]   = metrics.p99();

                    std::string line = response.dump() + "\n";
                    std::lock_guard<std::mutex> lock(clientWriteMutex);
                    write(fd, line.c_str(), line.size());
                }
            }
            catch(const std::exception& e){
                std::cerr << "Failed to parse command: " << e.what() << std::endl;
            }
        }
    }

    close(fd);
    clientFd = -1;
}

void NetworkServer::broadcastTrade(const Trade& trade){
    if(clientFd == -1) return;

    json j;
    j["event"]          = "trade";
    j["tradeId"]        = trade.tradeId;
    j["buyOrderId"]     = trade.buyOrderId;
    j["sellOrderId"]    = trade.sellOrderId;
    j["price"]          = toHuman(trade.price);
    j["quantity"]       = trade.quantity;
    j["timestamp"]      = trade.timestamp;
    j["isBuyerMaker"]   = trade.isBuyerMaker;

    std::string line = j.dump() + "\n";

    std::lock_guard<std::mutex> lock(clientWriteMutex);
    write(clientFd, line.c_str(), line.size());
}