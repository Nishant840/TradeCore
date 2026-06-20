const net = require('net')
const EventEmitter = require('events')

class EngineClient extends EventEmitter {
    constructor(host, port){
        super();
        this.host   = host;
        this.port   = port;
        this.socket = null;
        this.buffer = '';
    }

    connect() {
        this.socket = net.createConnection(this.port, this.host, ()=>{
            console.log(`Connected to engine at ${this.host}:${this.port}`);
            this.emit('connected');
        });

        this.socket.on('data', (chunk) => {
            this.buffer += chunk.toString();

            let newlineIndex;
            while((newlineIndex = this.buffer.indexOf('\n')) !== -1){
                const line  = this.buffer.slice(0,newlineIndex);
                this.buffer = this.buffer.slice(newlineIndex+1);

                if(line.length === 0) continue;

                try{
                    const message = JSON.parse(line);
                    this.emit('message', message);
                }
                catch (err){
                    console.error('Failed to parse engine message:', err.message);
                }
            }
        });

        this.socket.on('error', (err) => {
            console.error('Engine socket error:', err.message);
        });

        this.socket.on('close', ()=>{
            console.log('Engine connection closed');
            this.emit('disconnected');
        });
    }

    sendOrder(order){
        const message = {
            cmd: 'new_order',
            orderId: order.orderId,
            userId: order.userId,
            side: order.side,
            type: order.type,
            price: order.price,
            quantity: order.quantity,
        };
        this._send(message);
    }

    sendCancel(orderId){
        const message = {
            cmd: 'cancel',
            orderId: orderId,
        };
        this._send(message);
    }

    _send(message){
        if(!this.socket || this.socket.destroyed) {
            throw new Error('Engine socket not connected');
        }
        const line = JSON.stringify(message) + '\n';
        this.socket.write(line);
    }
}

module.exports = EngineClient;