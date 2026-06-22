import { Metrics } from "@/lib/api";

export default function MetricsPanel({ metrics }: { metrics: Metrics | null }) {
  const cells = [
    { label: "Total orders", value: metrics?.totalOrders ?? "—" },
    { label: "Total trades", value: metrics?.totalTrades ?? "—" },
    { label: "p50 latency", value: metrics ? `${metrics.latency.p50Micros}μs` : "—" },
    { label: "p99 latency", value: metrics ? `${metrics.latency.p99Micros}μs` : "—" },
  ];

  return (
    <div className="grid grid-cols-2 border-t" style={{ borderColor: "var(--color-border)" }}>
      {cells.map((cell, i) => (
        <div
          key={i}
          className="px-3 py-[7px] border-b"
          style={{
            borderColor: "var(--color-border)",
            borderRight: i % 2 === 0 ? "0.5px solid var(--color-border)" : "none",
          }}
        >
          <div className="text-[9px] uppercase tracking-wider" style={{ color: "var(--color-text-tertiary)" }}>
            {cell.label}
          </div>
          <div className="text-[14px] font-medium" style={{ color: "var(--color-text-primary)" }}>
            {cell.value}
          </div>
        </div>
      ))}
    </div>
  );
}