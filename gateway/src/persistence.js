const { pool } = require("./db")

async function insertOrder(order){
    const query = `
        INSERT INTO orders (id, user_id, side, type, price, quantity, status)
        VALUES ($1, $2, $3, $4, $5, $6, 'SUBMITTED')
        ON CONFLICT (id) DO NOTHING
    `;

    const values = [
        order.orderId,
        order.userId,
        order.side,
        order.type,
        order.type === 'MARKET'?null:order.price,
        order.quantity,
    ];

    try{
        await pool.query(query, values);
    }
    catch(err){
        console.error('Failed to persist order:', err.message);
    }
}

async function insertTrade(trade){
    const query = `
        INSERT INTO trades (id, buy_order_id, sell_order_id, price, quantity)
        VALUES ($1, $2, $3, $4, $5)
        ON CONFLICT (id) DO NOTHING
    `;
    
    const values = [
        trade.tradeId,
        trade.buyOrderId,
        trade.sellOrderId,
        trade.price,
        trade.quantity
    ];

    try{
        await pool.query(query, values);
    }
    catch(err){
        console.error('Failed to persist trade:', err.message);
    }
}

module.exports = { insertOrder, insertTrade};