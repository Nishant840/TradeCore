"use client";

import { useState } from "react";
import { placeOrder, Side, OrderType } from "@/lib/api";

export default function OrderForm({ onOrderPlaced }: { onOrderPlaced: () => void }) {
  const [side, setSide] = useState<Side>("BUY");
  const [type, setType] = useState<OrderType>("LIMIT");
  const [price, setPrice] = useState("100.00");
  const [quantity, setQuantity] = useState("1");
  const [submitting, setSubmitting] = useState(false);
  const [error, setError] = useState<string | null>(null);

  async function handleSubmit() {
    setSubmitting(true);
    setError(null);

    try {
      await placeOrder({
        userId: "demo-user",
        side,
        type,
        price: type === "LIMIT" ? parseFloat(price) : undefined,
        quantity: parseInt(quantity, 10),
      });
      onOrderPlaced();
    } catch (err) {
      setError(err instanceof Error ? err.message : "Failed to place order");
    } finally {
      setSubmitting(false);
    }
  }

  const total = type === "LIMIT" ? (parseFloat(price || "0") * parseInt(quantity || "0", 10)).toFixed(2) : "—";

  return (
    <div className="p-3 border-t" style={{ borderColor: "var(--color-border)" }}>
      <div className="flex mb-2">
        <button
          onClick={() => setSide("BUY")}
          className="flex-1 text-[12px] py-[5px] border"
          style={{
            borderColor: side === "BUY" ? "var(--color-green)" : "var(--color-border)",
            background: side === "BUY" ? "rgba(22,160,92,0.1)" : "transparent",
            color: side === "BUY" ? "var(--color-green)" : "var(--color-text-secondary)",
            borderRadius: "4px 0 0 4px",
          }}
        >
          Buy
        </button>
        <button
          onClick={() => setSide("SELL")}
          className="flex-1 text-[12px] py-[5px] border"
          style={{
            borderColor: side === "SELL" ? "var(--color-red)" : "var(--color-border)",
            background: side === "SELL" ? "rgba(226,75,74,0.1)" : "transparent",
            color: side === "SELL" ? "var(--color-red)" : "var(--color-text-secondary)",
            borderRadius: "0 4px 4px 0",
          }}
        >
          Sell
        </button>
      </div>

      <div className="flex gap-1.5 mb-2">
        {(["LIMIT", "MARKET"] as OrderType[]).map((t) => (
          <button
            key={t}
            onClick={() => setType(t)}
            className="text-[10px] px-2 py-[3px] border rounded-[3px]"
            style={{
              borderColor: type === t ? "var(--color-text-secondary)" : "var(--color-border)",
              background: type === t ? "var(--color-bg-secondary)" : "transparent",
              color: type === t ? "var(--color-text-primary)" : "var(--color-text-secondary)",
            }}
          >
            {t === "LIMIT" ? "Limit" : "Market"}
          </button>
        ))}
      </div>

      <div className="grid grid-cols-2 gap-2 mb-2">
        {type === "LIMIT" && (
          <div>
            <div className="text-[10px] mb-[3px]" style={{ color: "var(--color-text-secondary)" }}>
              Price
            </div>
            <input
              value={price}
              onChange={(e) => setPrice(e.target.value)}
              className="w-full text-[12px] px-2 py-[5px] rounded"
              style={{ background: "var(--color-bg-secondary)", border: "0.5px solid var(--color-border)", color: "var(--color-text-primary)" }}
            />
          </div>
        )}
        <div className={type === "MARKET" ? "col-span-2" : ""}>
          <div className="text-[10px] mb-[3px]" style={{ color: "var(--color-text-secondary)" }}>
            Quantity
          </div>
          <input
            value={quantity}
            onChange={(e) => setQuantity(e.target.value)}
            className="w-full text-[12px] px-2 py-[5px] rounded"
            style={{ background: "var(--color-bg-secondary)", border: "0.5px solid var(--color-border)", color: "var(--color-text-primary)" }}
          />
        </div>
      </div>

      <div className="flex justify-between text-[10px] mb-2" style={{ color: "var(--color-text-secondary)" }}>
        <span>Total</span>
        <span style={{ color: "var(--color-text-primary)" }}>{total}</span>
      </div>

      {error && <div className="text-[10px] mb-2" style={{ color: "var(--color-red)" }}>{error}</div>}

      <button
        onClick={handleSubmit}
        disabled={submitting}
        className="w-full py-2 text-[13px] rounded font-medium"
        style={{
          background: side === "BUY" ? "var(--color-green)" : "var(--color-red)",
          color: "#fff",
          opacity: submitting ? 0.6 : 1,
        }}
      >
        {submitting ? "Placing..." : `Place ${side.toLowerCase()} order`}
      </button>
    </div>
  );
}