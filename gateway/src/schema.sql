CREATE TABLE IF NOT EXISTS orders (
    id          BIGINT PRIMARY KEY,
    user_id     TEXT NOT NULL,
    side        TEXT NOT NULL CHECK (side IN ('BUY', 'SELL')),
    type        TEXT NOT NULL CHECK (type IN ('LIMIT', 'MARKET')),
    price       NUMERIC,
    quantity    BIGINT NOT NULL,
    status      TEXT NOT NULL DEFAULT 'SUBMITTED',
    created_at  TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS trades (
    id              BIGINT PRIMARY KEY,
    buy_order_id    BIGINT NOT NULL,
    sell_order_id   BIGINT NOT NULL,
    price           NUMERIC NOT NULL,
    quantity        BIGINT NOT NULL,
    executed_at     TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_trades_executed_at ON trades (executed_at DESC);
CREATE INDEX IF NOT EXISTS idx_orders_user_id ON orders (user_id);