import { BookSnapshot } from "@/lib/api";

export default function OrderBook({ book }: { book: BookSnapshot }) {
  const maxAskQty = Math.max(...book.asks.map((a) => a.quantity), 1);
  const maxBidQty = Math.max(...book.bids.map((b) => b.quantity), 1);

  const bestBid = book.bids[0]?.price;
  const bestAsk = book.asks[0]?.price;
  const spread = bestAsk && bestBid ? (bestAsk - bestBid).toFixed(2) : "—";

  return (
    <div className="flex flex-col h-full border-r" style={{ borderColor: "var(--color-border)" }}>
      <div className="px-3 py-2 text-[10px] uppercase tracking-wider border-b"
           style={{ color: "var(--color-text-tertiary)", borderColor: "var(--color-border)" }}>
        Order book
      </div>

      <div className="flex flex-col justify-end flex-1">
        {[...book.asks].reverse().map((level, i) => (
          <Row key={`ask-${i}`} price={level.price} qty={level.quantity} max={maxAskQty} side="ask" />
        ))}
      </div>

      <div className="px-3 py-1 text-[10px] text-center border-y"
           style={{ color: "var(--color-text-tertiary)", borderColor: "var(--color-border)", background: "var(--color-bg-secondary)" }}>
        Spread {spread}
      </div>

      <div className="flex flex-col">
        {book.bids.map((level, i) => (
          <Row key={`bid-${i}`} price={level.price} qty={level.quantity} max={maxBidQty} side="bid" />
        ))}
      </div>
    </div>
  );
}

function Row({ price, qty, max, side }: { price: number; qty: number; max: number; side: "ask" | "bid" }) {
  const pct = Math.min((qty / max) * 100, 100);
  const color = side === "ask" ? "var(--color-red)" : "var(--color-green)";

  return (
    <div className="relative flex justify-between items-center px-3 py-[2px] text-[11px]">
      <div className="absolute top-0 bottom-0 right-0 opacity-10" style={{ width: `${pct}%`, background: color }} />
      <span style={{ color }}>{price.toFixed(2)}</span>
      <span style={{ color: "var(--color-text-secondary)" }}>{qty}</span>
    </div>
  );
}