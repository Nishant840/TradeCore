# Stage 1: Build the C++ Matching Engine
FROM ubuntu:22.04 AS cpp-builder

RUN apt-get update && apt-get install -y cmake g++ make && rm -rf /var/lib/apt/lists/*

WORKDIR /app/matching-engine
COPY matching-engine/ ./

RUN mkdir -p build && cd build && cmake .. && make

# Stage 2: Runtime image — Node gateway + compiled C++ binary
FROM node:18-bullseye-slim

RUN apt-get update && apt-get install -y libstdc++6 libc6 netcat-openbsd && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=cpp-builder /app/matching-engine/build/tradecore_engine /app/tradecore_engine

WORKDIR /app/gateway
COPY gateway/package*.json ./
RUN npm install --production
COPY gateway/ ./

WORKDIR /app
COPY start.sh ./start.sh
RUN chmod +x start.sh

EXPOSE 4000

CMD ["./start.sh"]