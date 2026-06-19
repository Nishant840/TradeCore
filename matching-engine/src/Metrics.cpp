#include "../include/Metrics.h"

void Metrics::recordOrderLatecy(uint64_t microseconds){
    latencies.push_back(microseconds);
    totalOrders.fetch_add(1, std::memory_order_relaxed);
}

void Metrics::incrementTradeCount(){
    totalTrades.fetch_add(1, std::memory_order_relaxed);
}

uint64_t Metrics::getTotalOrders() const {
    return totalOrders.load(std::memory_order_relaxed);
}

uint64_t Metrics::getTotalTrades() const {
    return totalTrades.load(std::memory_order_relaxed);
}

uint64_t Metrics::percentile(double p) const {
    if(latencies.empty()){
        return 0;
    }

    std::vector<uint64_t> sorted = latencies;
    std::sort(sorted.begin(),sorted.end());

    size_t index = static_cast<size_t>(p * (sorted.size() - 1));
    return sorted[index];
}

uint64_t Metrics::p50() const {
    return percentile(0.50);
}

uint64_t Metrics::p95() const {
    return percentile(0.95);
}

uint64_t Metrics::p99() const {
    return percentile(0.99);
}