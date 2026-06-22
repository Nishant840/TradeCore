"use client";

import { useDashboardData } from "@/lib/hooks";
import OrderBook from "@/components/OrderBook";
import TradeFeed from "@/components/TradeFeed";
import OrderForm from "@/components/OrderForm";
import MetricsPanel from "@/components/MetricsPanel";
import PriceChart from "@/components/PriceChart";

export default function Home() {
  const { book, metrics, trades, liveTrades, connected, refresh } = useDashboardData();

  return (
    <div className="min-h-screen flex flex-col" style={{ background: "var(--color-bg-tertiary)" }}>
      <div
        className="flex items-center gap-4 px-4 py-2.5 border-b shrink-0"
        style={{ borderColor: "var(--color-border)", background: "var(--color-bg-primary)" }}
      >
        <span className="text-[15px] font-medium">TradeCore</span>
        <span className="text-[11px]" style={{ color: "var(--color-text-secondary)" }}>
          BTC / USDT
        </span>
        <div className="flex-1" />
        <span
          className="w-1.5 h-1.5 rounded-full"
          style={{ background: connected ? "var(--color-green)" : "var(--color-text-tertiary)" }}
        />
        <span className="text-[11px]" style={{ color: "var(--color-text-secondary)" }}>
          {connected ? "Live" : "Disconnected"}
        </span>
      </div>

      <div className="flex-1 grid grid-cols-1 md:grid-cols-[220px_1fr_240px] overflow-y-auto md:overflow-hidden">
        <div className="md:h-full border-b md:border-b-0">
          <OrderBook book={book} />
        </div>

        <div
          className="flex flex-col border-b md:border-b-0 md:border-r"
          style={{ borderColor: "var(--color-border)" }}
        >
          <div className="hidden md:flex flex-1 items-center justify-center">
            <PriceChart trades={trades} liveTrades={liveTrades} />
          </div>
          <OrderForm onOrderPlaced={refresh} />
        </div>

        <div
          className="flex flex-col min-h-[300px] md:min-h-0 md:h-full"
          style={{ background: "var(--color-bg-primary)" }}
        >
          <div
            className="px-3 py-1.5 text-[10px] uppercase tracking-wider border-b shrink-0"
            style={{ color: "var(--color-text-tertiary)", borderColor: "var(--color-border)" }}
          >
            Recent trades
          </div>
          <TradeFeed trades={liveTrades} />
          <MetricsPanel metrics={metrics} />
        </div>
      </div>
    </div>
  );
}