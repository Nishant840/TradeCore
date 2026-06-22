const API_BASE = process.env.NEXT_PUBLIC_API_URL || "http://localhost:4000";
const WS_BASE = process.env.NEXT_PUBLIC_WS_URL || "ws://localhost:4000";

export type Side = "BUY" | "SELL";
export type OrderType = "LIMIT" | "MARKET";

export interface PriceLevel {
  price: number;
  quantity: number;
}

export interface BookSnapshot {
  bids: PriceLevel[];
  asks: PriceLevel[];
}

export interface Trade {
  id: string;
  buy_order_id: string;
  sell_order_id: string;
  price: string;
  quantity: string;
  executed_at: string;
}

export interface TradeEvent {
  event: "trade";
  tradeId: number;
  buyOrderId: number;
  sellOrderId: number;
  price: number;
  quantity: number;
  timestamp: number;
  isBuyerMaker: boolean;
}

export interface Metrics {
  totalOrders: number;
  totalTrades: number;
  latency: {
    p50Micros: number;
    p95Micros: number;
    p99Micros: number;
  };
}

export async function getBook(depth = 5): Promise<BookSnapshot> {
  const res = await fetch(`${API_BASE}/book?depth=${depth}`);
  if (!res.ok) throw new Error("Failed to fetch order book");
  return res.json();
}

export async function getTrades(limit = 20): Promise<{ trades: Trade[] }> {
  const res = await fetch(`${API_BASE}/trades?limit=${limit}`);
  if (!res.ok) throw new Error("Failed to fetch trades");
  return res.json();
}

export async function getMetrics(): Promise<Metrics> {
  const res = await fetch(`${API_BASE}/metrics`);
  if (!res.ok) throw new Error("Failed to fetch metrics");
  return res.json();
}

export async function placeOrder(order: {
  userId: string;
  side: Side;
  type: OrderType;
  price?: number;
  quantity: number;
}): Promise<{ status: string; orderId: number }> {
  const res = await fetch(`${API_BASE}/order`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(order),
  });
  const data = await res.json();
  if (!res.ok) throw new Error(data.error || "Failed to place order");
  return data;
}

export function connectTradeFeed(onTrade: (trade: TradeEvent) => void): WebSocket {
  const ws = new WebSocket(`${WS_BASE}/ws`);

  ws.onmessage = (event) => {
    try {
      const data = JSON.parse(event.data);
      if (data.event === "trade") {
        onTrade(data as TradeEvent);
      }
    } catch (err) {
      console.error("Failed to parse WS message:", err);
    }
  };

  return ws;
}