"use client";

import { TradeEvent } from "@/lib/api";

export default function TradeFeed({ trades }: { trades: TradeEvent[] }) {
  if (trades.length === 0) {
    return (
      <div className="flex-1 flex items-center justify-center px-4 text-center">
        <span className="text-[11px]" style={{ color: "var(--color-text-tertiary)" }}>
          No trades yet — place an order to see it match
        </span>
      </div>
    );
  }

  return (
    <div className="flex-1 overflow-hidden flex flex-col">
      {trades.map((trade) => {
        const isBuyAggressor = !trade.isBuyerMaker;
        const time = new Date(trade.timestamp / 1000).toLocaleTimeString();

        return (
          <div
            key={trade.tradeId}
            className="flex justify-between items-center px-3 py-[2px] text-[11px] font-mono trade-row"
          >
            <span style={{ color: isBuyAggressor ? "var(--color-green)" : "var(--color-red)" }}>
              {trade.price.toFixed(2)}
            </span>
            <span style={{ color: "var(--color-text-secondary)" }}>{trade.quantity}</span>
            <span style={{ color: "var(--color-text-tertiary)" }}>{time}</span>
          </div>
        );
      })}
    </div>
  );
}