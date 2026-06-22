"use client";

import { useEffect, useRef } from "react";
import { createChart, ColorType, AreaSeries } from "lightweight-charts";
import { Trade, TradeEvent } from "@/lib/api";

export default function PriceChart({ trades, liveTrades }: { trades: Trade[], liveTrades: TradeEvent[] }) {
  const chartContainerRef = useRef<HTMLDivElement>(null);
  const chartRef = useRef<any>(null);
  const seriesRef = useRef<any>(null);

  useEffect(() => {
    if (!chartContainerRef.current) return;

    const chart = createChart(chartContainerRef.current, {
      layout: {
        background: { type: ColorType.Solid, color: "transparent" },
        textColor: "var(--color-text-secondary)",
        attributionLogo: false,
      },
      grid: {
        vertLines: { color: "var(--color-border)" },
        horzLines: { color: "var(--color-border)" },
      },
      timeScale: {
        timeVisible: true,
        secondsVisible: true,
      },
      width: chartContainerRef.current.clientWidth,
      height: chartContainerRef.current.clientHeight,
    });

    const series = chart.addSeries(AreaSeries, {
      lineColor: "var(--color-green)",
      topColor: "rgba(22, 160, 92, 0.4)",
      bottomColor: "rgba(22, 160, 92, 0)",
    });

    chartRef.current = chart;
    seriesRef.current = series;

    const handleResize = () => {
      if (chartContainerRef.current) {
        chart.applyOptions({
          width: chartContainerRef.current.clientWidth,
          height: chartContainerRef.current.clientHeight,
        });
      }
    };
    window.addEventListener("resize", handleResize);

    return () => {
      window.removeEventListener("resize", handleResize);
      chart.remove();
    };
  }, []);

  useEffect(() => {
    if (!seriesRef.current) return;

    const tzOffset = new Date().getTimezoneOffset() * 60;

    const historicalData = trades.map(t => ({
      time: Math.floor(new Date(t.executed_at).getTime() / 1000) - tzOffset,
      value: parseFloat(t.price),
    }));

    const liveData = liveTrades.map(t => ({
      // timestamp is in microseconds from the C++ engine
      time: Math.floor(t.timestamp / 1000000) - tzOffset,
      value: t.price,
    }));

    const allData = [...historicalData, ...liveData]
      .sort((a, b) => a.time - b.time)
      .reduce((acc, current) => {
        if (acc.length === 0) {
          acc.push(current);
        } else {
          const last = acc[acc.length - 1];
          if (current.time > last.time) {
            acc.push(current);
          } else {
            // override with the latest value for the same second
            acc[acc.length - 1] = current;
          }
        }
        return acc;
      }, [] as {time: number, value: number}[]);

    if (allData.length > 0) {
      seriesRef.current.setData(allData);
    }
  }, [trades, liveTrades]);

  return (
    <div className="w-full h-full flex flex-col">
      <div className="flex-1 w-full" ref={chartContainerRef} />
    </div>
  );
}
