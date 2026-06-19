#include "../include/MatchingEngine.h"
#include <chrono>
#include <stdexcept>

MatchingEngine::MatchingEngine(TradeCallback callback)
    : onTrade(callback), nextTradeId(1) {}

void MatchingEngine::processOrder(Order& order){
    if(order.type == OrderType::LIMIT){
        matchLimit(order);
    }
    else{
        matchMarket(order);
    }

    if(order.type == OrderType::LIMIT && order.quantity > 0){
        book.addOrder(order);
    }
}

void MatchingEngine::cancelOrder(uint64_t orderId){
    book.cancelOrder(orderId);
}

const OrderBook& MatchingEngine::getOrderBook() const {
    return book;
}

void MatchingEngine::matchLimit(Order& order){
    if(order.side == Side::BUY){
        while(order.quantity > 0 && book.hasAsks()){
            int64_t bestAsk = book.bestAsk();
            if(order.price < bestAsk) break;

            auto& level = book.getAsks().find(bestAsk)->second;
            while(order.quantity > 0 && !level.empty()){
                OrderNode& node     = level.front();
                Order& resting      = node.order;
                uint64_t traded     = std::min(order.quantity, resting.quantity);

                Trade t = createTrade(order, resting, bestAsk, traded);
                onTrade(t);

                order.quantity      -= traded;
                resting.quantity    -= traded;

                if(resting.quantity == 0){
                    level.erase(level.begin());
                }
            }

            if(level.empty()){
                book.getAsks().erase(bestAsk);
            }
        }
    }
    else{
        while(order.quantity > 0 && book.hasBids()){
            int64_t bestBid = book.bestBid();
            if(order.price > bestBid) break;

            auto& level = book.getBids().find(bestBid)->second;
            while(order.quantity > 0 && !level.empty()){
                OrderNode& node = level.front();
                Order& resting  = node.order;
                uint64_t traded = std::min(order.quantity, resting.quantity);

                Trade t = createTrade(resting, order, bestBid, traded);
                onTrade(t);

                order.quantity   -= traded;
                resting.quantity -= traded;

                if(resting.quantity == 0){
                    level.erase(level.begin());
                }
            }

            if(level.empty()){
                book.getBids().erase(bestBid);
            }
        }
    }
}

void MatchingEngine::matchMarket(Order& order){
    if(order.side == Side::BUY){
        while(order.quantity > 0 && book.hasAsks()){
            int64_t bestAsk = book.bestAsk();
            auto& level = book.getAsks().find(bestAsk)->second;

            while(order.quantity > 0 && !level.empty()){
                OrderNode& node = level.front();
                Order& resting  = node.order;
                uint64_t traded = std::min(order.quantity, resting.quantity);

                Trade t = createTrade(order, resting, bestAsk, traded);
                onTrade(t);

                order.quantity -= traded;
                resting.quantity -= traded;

                if(resting.quantity == 0){
                    level.erase(level.begin());
                }
            }

            if(level.empty()){
                book.getAsks().erase(bestAsk);
            }
        }
    }
    else{
        while(order.quantity > 0 && book.hasBids()){
            int64_t bestBid = book.bestBid();
            auto& level    = book.getBids().find(bestBid)->second;

            while(order.quantity > 0 && !level.empty()){
                OrderNode& node = level.front();
                Order& resting  = node.order;
                uint64_t traded = std::min(order.quantity, resting.quantity);

                Trade t = createTrade(resting, order, bestBid, traded);
                onTrade(t);

                order.quantity -= traded;
                resting.quantity -= traded;

                if(resting.quantity == 0){
                    level.erase(level.begin());
                }
            }
            if(level.empty()){
                book.getBids().erase(bestBid);
            }
        }
    }
}

Trade MatchingEngine::createTrade( 
    const Order& buy,
    const Order& sell,
    int64_t price,
    uint64_t quantity){
        auto now = std::chrono::high_resolution_clock::now();
        uint64_t ts = std::chrono::duration_cast<std::chrono::microseconds>(
            now.time_since_epoch()
        ).count();

        Trade t;
        t.tradeId       = nextTradeId++;
        t.buyOrderId    = buy.orderId;
        t.sellOrderId   = sell.orderId;
        t.price         = price;
        t.quantity      = quantity;
        t.timestamp     = ts;

        return t;
}