#pragma once

#include<cstdint>
#include<vector>
#include<algorithm>
#include<atomic>

class Metrics{
  public:
    void recordOrderLatecy(uint64_t microseconds);
    void incrementTradeCount();

    uint64_t getTotalOrders() const;
    uint64_t getTotalTrades() const;

    uint64_t p50() const;
    uint64_t p95() const;
    uint64_t p99() const;

  private:
    std::vector<uint64_t> latencies;
    std::atomic<uint64_t> totalOrders{0};
    std::atomic<uint64_t> totalTrades{0};

    uint64_t percentile(double p) const;
};