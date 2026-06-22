"use client";

import { useState, useEffect, useCallback, useRef } from "react";
import { getBook, getMetrics, getTrades, connectTradeFeed, BookSnapshot, Metrics, Trade, TradeEvent } from "./api";

export function useDashboardData() {
  const [book, setBook] = useState<BookSnapshot>({ bids: [], asks: [] });
  const [metrics, setMetrics] = useState<Metrics | null>(null);
  const [trades, setTrades] = useState<Trade[]>([]);
  const [liveTrades, setLiveTrades] = useState<TradeEvent[]>([]);
  const [connected, setConnected] = useState(false);
  const refreshTimeoutRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const refreshInFlightRef = useRef(false);

  const refresh = useCallback(async () => {
    if (refreshInFlightRef.current) return;
    refreshInFlightRef.current = true;

    try {
      const [bookData, metricsData, tradesData] = await Promise.all([
        getBook(),
        getMetrics(),
        getTrades(20),
      ]);
      setBook(bookData);
      setMetrics(metricsData);
      setTrades(tradesData.trades);
    } catch (err) {
      console.error("Failed to refresh dashboard data:", err);
    } finally {
      refreshInFlightRef.current = false;
    }
  }, []);

  const scheduleRefresh = useCallback(() => {
    if (refreshTimeoutRef.current) return;
    refreshTimeoutRef.current = setTimeout(() => {
      refreshTimeoutRef.current = null;
      refresh();
    }, 250);
  }, [refresh]);

  useEffect(() => {
    refresh();

    const ws = connectTradeFeed((trade) => {
      setLiveTrades((prev) => [trade, ...prev].slice(0, 30));
      scheduleRefresh();
    });

    ws.onopen = () => setConnected(true);
    ws.onclose = () => setConnected(false);

    return () => {
      ws.close();
      if (refreshTimeoutRef.current) clearTimeout(refreshTimeoutRef.current);
    };
  }, [refresh, scheduleRefresh]);

  return { book, metrics, trades, liveTrades, connected, refresh };
}