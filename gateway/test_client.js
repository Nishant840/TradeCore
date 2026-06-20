const EngineClient = require('./src/engineClient');

const client = new EngineClient('localhost', 9000);

client.on('connected', () => {
    console.log('Test client connected to engine');

    const sellOrder = {
        orderId: 101,
        userId: 'alice',
        side: 'SELL',
        type: 'LIMIT',
        price: 100.50,
        quantity: 5,
    };

    const buyOrder = {
        orderId: 102,
        userId: 'bob',
        side: 'BUY',
        type: 'LIMIT',
        price: 100.50,
        quantity: 5,
    };

    console.log('Sending sell order:', sellOrder);
    client.sendOrder(sellOrder);

    setTimeout(() => {
        console.log('Sending buy order:', buyOrder);
        client.sendOrder(buyOrder);
    }, 500);
});

client.on('message', (msg) => {
    console.log('Received from engine:', msg);
});

client.on('disconnected', () => {
    console.log('Test client disconnected');
});

client.connect();