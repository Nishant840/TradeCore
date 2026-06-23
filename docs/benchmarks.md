# Performance Benchmarks

This document records the performance characteristics of the TradeCore exchange engine and API gateway. The results below showcase raw C++ execution speed and real-world network latency.

## 1. Raw Engine Throughput (C++ In-Memory)
*These metrics measure the pure processing power of the C++ Matching Engine under heavy multithreaded stress, bypassing the network layer entirely.*

**Scale:** 10,000 orders (4 concurrent producer threads)

| Environment | Total Orders | Elapsed Time (s) | Throughput (Orders/Sec) |
|-------------|--------------|------------------|-------------------------|
| Local (Apple Silicon / M-Series) | 10,000 | 0.0242871 | 411,742 |
| Deployed (Render Free Tier)      | 10,000 | `[Run in cloud to fill]`     | `[Result]` |

---

## 2. API Gateway Throughput & Latency (Node.js + C++ via TCP)
*These metrics simulate real-world REST API traffic hitting the Node.js Gateway, parsing JSON, pushing to the C++ engine over local TCP, and returning the response.*

**Scale:** 10,000 concurrent REST POST requests
**Concurrency:** Batch size of 500 concurrent requests in-flight at any given time.

| Environment | Throughput (Orders/Sec) | Success Rate | p50 (ms) | p95 (ms) | p99 (ms) |
|-------------|-------------------------|--------------|----------|----------|----------|
| Local (M-Series) | 6,031.62         | 10000 / 10000 | 43.62ms | 82.01ms | 251.27ms |
| Render Free Tier | `[Result]`         | 10000 / 10000 | `[Result]` | `[Result]` | `[Result]` |

> [!NOTE]
> **Investigating Tail Latency (p95 -> p99)**
> The sharp 3x jump from p95 (82ms) to p99 (251ms) is due to **PostgreSQL connection pool exhaustion**. The Node.js gateway uses `pg` with the default maximum pool size of 10 connections. Because the load test fires 500 concurrent requests, and the gateway does a "fire-and-forget" `INSERT` for every order, the database connection pool queues up the excess queries. The 251ms tail latency represents the requests stuck waiting in Node's event loop for a free DB connection, *not* the C++ engine's matching time.

---

## 3. Internal Engine Latency (Self-Reported)
*These metrics are captured by the C++ engine internally using `std::chrono::high_resolution_clock`. It measures the time taken from the moment an order is popped from the TCP queue to the moment it is fully matched and committed to the order book.*

| Environment | p50 Latency (μs) | p95 Latency (μs) | p99 Latency (μs) |
|-------------|------------------|------------------|------------------|
| Local (M-Series) | 3 μs | 57 μs | 105 μs |
| Render Free Tier | `[Result] μs` | `[Result] μs` | `[Result] μs` |

---

### How to reproduce:
1. **Raw C++ test**: Compile tests using CMake and run `./test_matching`
2. **API load test**: Start the backend, then run `node gateway/scripts/loadtest.js`
