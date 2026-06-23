const API_BASE = process.env.API_URL || "http://localhost:4000";

const NUM_ORDERS = 10000;
const BATCH_SIZE = 500;

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function runLoadTest() {
    console.log(`Starting load test against ${API_BASE}`);
    console.log(`Total Orders: ${NUM_ORDERS} (Batches of ${BATCH_SIZE})`);
    
    let latencies = [];
    
    // Generate order payloads
    const orders = [];
    for (let i = 0; i < NUM_ORDERS; i++) {
        orders.push({
            userId: `loadtester-${i % 100}`,
            side: i % 2 === 0 ? "BUY" : "SELL",
            type: "LIMIT",
            price: 100 + (Math.random() * 10 - 5), // price around 100
            quantity: 1
        });
    }

    const startTime = performance.now();
    let successCount = 0;
    let errorCount = 0;
    let errorReasons = {};

    for (let i = 0; i < NUM_ORDERS; i += BATCH_SIZE) {
        const batch = orders.slice(i, i + BATCH_SIZE);
        
        const promises = batch.map(async (order) => {
            const reqStart = performance.now();
            try {
                const res = await fetch(`${API_BASE}/order`, {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify(order)
                });
                
                if (res.ok) {
                    successCount++;
                    latencies.push(performance.now() - reqStart);
                } else {
                    errorCount++;
                    errorReasons[`HTTP ${res.status}`] = (errorReasons[`HTTP ${res.status}`] || 0) + 1;
                }
            } catch (err) {
                errorCount++;
                const msg = err.message || 'Unknown Error';
                errorReasons[msg] = (errorReasons[msg] || 0) + 1;
            }
        });

        await Promise.all(promises);
    }

    const endTime = performance.now();
    const totalTimeSec = (endTime - startTime) / 1000;
    const throughput = NUM_ORDERS / totalTimeSec;

    latencies.sort((a, b) => a - b);
    const p50 = latencies[Math.floor(latencies.length * 0.50)] || 0;
    const p95 = latencies[Math.floor(latencies.length * 0.95)] || 0;
    const p99 = latencies[Math.floor(latencies.length * 0.99)] || 0;

    console.log("\n--- API GATEWAY BENCHMARK ---");
    console.log(`Throughput     : ${throughput.toFixed(2)} orders/sec`);
    console.log(`Success Rate   : ${successCount} / ${NUM_ORDERS}`);
    console.log(`Errors         : ${errorCount}`);
    if (errorCount > 0) {
        console.log(`Error Reasons  :`);
        for (const [reason, count] of Object.entries(errorReasons)) {
            console.log(`  - ${reason}: ${count}`);
        }
    }
    console.log(`Client Latency : p50=${p50.toFixed(2)}ms, p95=${p95.toFixed(2)}ms, p99=${p99.toFixed(2)}ms`);
    
    console.log("\nFetching Engine Metrics...");
    await sleep(1000); // give engine a sec to catch up if queued

    try {
        const metricsRes = await fetch(`${API_BASE}/metrics`);
        const metrics = await metricsRes.json();
        
        console.log("\n--- INTERNAL ENGINE BENCHMARK ---");
        console.log(`Total Orders Processed: ${metrics.totalOrders}`);
        console.log(`Total Trades Executed : ${metrics.totalTrades}`);
        console.log(`Engine Latency (μs)   : p50=${metrics.latency.p50Micros}μs, p95=${metrics.latency.p95Micros}μs, p99=${metrics.latency.p99Micros}μs`);
    } catch(err) {
        console.error("Failed to fetch engine metrics:", err.message);
    }
}

runLoadTest();
