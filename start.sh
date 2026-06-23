#!/bin/bash
set -e

echo "Starting C++ matching engine..."
/app/tradecore_engine &
ENGINE_PID=$!

echo "Waiting for engine to bind to port 9000..."
for i in $(seq 1 20); do
    if nc -z localhost 9000; then
        echo "Engine is ready."
        break
    fi
    sleep 0.5
done

echo "Starting Node.js gateway..."
cd /app/gateway && node index.js &
GATEWAY_PID=$!

wait -n "$ENGINE_PID" "$GATEWAY_PID"
EXIT_CODE=$?

echo "A process exited (code $EXIT_CODE). Shutting down container."
kill "$ENGINE_PID" "$GATEWAY_PID" 2>/dev/null
exit "$EXIT_CODE"